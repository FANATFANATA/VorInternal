#include <Windows.h>
#include <TlHelp32.h>
#include <shellapi.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
    constexpr DWORD INJECTION_ACCESS = PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
                                       PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ;
    constexpr DWORD MAX_PROCESS_WAIT_TIME_MS = 10000;
    constexpr DWORD PROCESS_WAIT_INTERVAL_MS = 500;

    struct handle_closer
    {
        void operator()(HANDLE h) const noexcept
        {
            if (h && h != INVALID_HANDLE_VALUE)
            {
                ::CloseHandle(h);
            }
        }
    };

    using unique_handle = std::unique_ptr<void, handle_closer>;

    unique_handle make_handle(HANDLE h) noexcept
    {
        return unique_handle(h);
    }

    std::wstring get_exe_directory()
    {
        DWORD length = ::GetModuleFileNameW(nullptr, nullptr, 0);
        if (length == 0)
            return {};

        std::wstring buffer(length, L'\0');
        DWORD result = ::GetModuleFileNameW(nullptr, buffer.data(), length);
        if (result == 0 || result >= length)
            return {};

        buffer.resize(result);
        std::size_t pos = buffer.find_last_of(L"\\/");
        if (pos != std::wstring::npos)
        {
            return buffer.substr(0, pos);
        }
        return {};
    }

    DWORD find_process_id(const std::wstring &process_name) noexcept
    {
        unique_handle snapshot = make_handle(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
        if (!snapshot)
            return 0;

        PROCESSENTRY32W entry{};
        entry.dwSize = sizeof(PROCESSENTRY32W);
        if (!::Process32FirstW(snapshot.get(), &entry))
            return 0;

        do
        {
            if (_wcsicmp(process_name.c_str(), entry.szExeFile) == 0)
                return entry.th32ProcessID;
        } while (::Process32NextW(snapshot.get(), &entry));

        return 0;
    }

    DWORD wait_for_process(const std::wstring &process_name) noexcept
    {
        DWORD elapsed = 0;
        while (elapsed < MAX_PROCESS_WAIT_TIME_MS)
        {
            DWORD pid = find_process_id(process_name);
            if (pid != 0)
                return pid;
            ::Sleep(PROCESS_WAIT_INTERVAL_MS);
            elapsed += PROCESS_WAIT_INTERVAL_MS;
        }
        return 0;
    }

    bool file_exists(const std::wstring &path) noexcept
    {
        DWORD attribs = ::GetFileAttributesW(path.c_str());
        return attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY);
    }

    DWORD inject_dll(DWORD process_id, const std::wstring &dll_path) noexcept
    {
        unique_handle process = make_handle(::OpenProcess(INJECTION_ACCESS, FALSE, process_id));
        if (!process)
            return ::GetLastError();

        const std::size_t buffer_size = (dll_path.size() + 1) * sizeof(wchar_t);
        LPVOID remote_memory = ::VirtualAllocEx(process.get(), nullptr, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!remote_memory)
            return ::GetLastError();

        auto free_remote = [&]() noexcept
        {
            ::VirtualFreeEx(process.get(), remote_memory, 0, MEM_RELEASE);
        };

        if (!::WriteProcessMemory(process.get(), remote_memory, dll_path.c_str(), buffer_size, nullptr))
        {
            DWORD err = ::GetLastError();
            free_remote();
            return err;
        }

        HMODULE kernel32 = ::GetModuleHandleW(L"kernel32.dll");
        if (!kernel32)
        {
            free_remote();
            return ::GetLastError();
        }

        auto load_library_w = reinterpret_cast<LPTHREAD_START_ROUTINE>(::GetProcAddress(kernel32, "LoadLibraryW"));
        if (!load_library_w)
        {
            free_remote();
            return ::GetLastError();
        }

        unique_handle thread = make_handle(::CreateRemoteThread(process.get(), nullptr, 0, load_library_w, remote_memory, 0, nullptr));
        if (!thread)
        {
            DWORD err = ::GetLastError();
            free_remote();
            return err;
        }

        DWORD wait_result = ::WaitForSingleObject(thread.get(), INFINITE);
        if (wait_result != WAIT_OBJECT_0)
        {
            DWORD err = ::GetLastError();
            free_remote();
            return err == 0 ? ERROR_INVALID_HANDLE : err;
        }

        DWORD exit_code = 0;
        if (!::GetExitCodeThread(thread.get(), &exit_code))
        {
            DWORD err = ::GetLastError();
            free_remote();
            return err;
        }

        free_remote();
        return (exit_code != 0) ? ERROR_SUCCESS : ERROR_DLL_INIT_FAILED;
    }

    std::wstring resolve_absolute_path(const std::wstring &relative_path)
    {
        DWORD length = ::GetFullPathNameW(relative_path.c_str(), 0, nullptr, nullptr);
        if (length == 0)
            return {};

        std::wstring buffer(length, L'\0');
        DWORD result = ::GetFullPathNameW(relative_path.c_str(), length, buffer.data(), nullptr);
        if (result == 0 || result >= length)
            return {};

        buffer.resize(result);
        return buffer;
    }

    DWORD run_injector()
    {
        if (sizeof(void *) != 8)
        {
            std::wcerr << L"Error: Injector must be compiled as x64.\n";
            return 1;
        }

        int argc = 0;
        LPWSTR *argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
        if (!argv)
            return 1;

        std::wstring process_name = L"cs2.exe";
        std::wstring dll_path = L"VorInternal.dll";

        if (argc >= 2)
            process_name = argv[1];
        if (argc >= 3)
            dll_path = argv[2];

        if (process_name.empty() || dll_path.empty())
        {
            std::wcerr << L"Invalid arguments.\n";
            ::LocalFree(argv);
            return 1;
        }

        std::wstring absolute_dll_path = resolve_absolute_path(dll_path);
        if (absolute_dll_path.empty() || !file_exists(absolute_dll_path))
        {
            std::wstring exe_dir = get_exe_directory();
            if (!exe_dir.empty())
            {
                std::wstring fallback_path = exe_dir + L"\\" + dll_path;
                if (file_exists(fallback_path))
                {
                    absolute_dll_path = resolve_absolute_path(fallback_path);
                }
            }
        }

        if (absolute_dll_path.empty() || !file_exists(absolute_dll_path))
        {
            std::wcerr << L"DLL not found: " << dll_path << L"\n";
            ::LocalFree(argv);
            return 1;
        }

        std::wcout << L"Waiting for " << process_name << L"...\n";
        const DWORD process_id = wait_for_process(process_name);
        if (process_id == 0)
        {
            std::wcerr << L"Process not found: " << process_name << L"\n";
            ::LocalFree(argv);
            return 1;
        }

        DWORD err = inject_dll(process_id, absolute_dll_path);
        if (err == ERROR_SUCCESS)
        {
            std::wcout << L"Injected successfully into PID " << process_id << L"\n";
            ::LocalFree(argv);
            return 0;
        }

        std::wcerr << L"Injection failed (error: " << err << L")\n";
        ::LocalFree(argv);
        return 1;
    }
}

int main()
{
    ::SetConsoleOutputCP(CP_UTF8);
    return run_injector();
}
