#include <Windows.h>
#include <TlHelp32.h>
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

    auto handle_closer = [](HANDLE h) noexcept
    {
        if (h != nullptr && h != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(h);
        }
    };

    using SmartHandle = std::shared_ptr<void>;

    SmartHandle make_smart_handle(HANDLE h) noexcept
    {
        return SmartHandle(h, handle_closer);
    }

    bool is_valid_handle(HANDLE h) noexcept
    {
        return h != nullptr && h != INVALID_HANDLE_VALUE;
    }

    std::wstring utf8_to_wide(const char *str)
    {
        if (!str)
            return {};
        const int size = ::MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
        if (size <= 0)
            return {};
        std::wstring result(static_cast<std::size_t>(size - 1), L'\0');
        ::MultiByteToWideChar(CP_UTF8, 0, str, -1, result.data(), size);
        return result;
    }

    std::wstring get_exe_directory()
    {
        wchar_t buffer[MAX_PATH] = {};
        DWORD length = ::GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (length == 0 || length >= MAX_PATH)
            return {};
        std::wstring path(buffer, length);
        std::size_t pos = path.find_last_of(L"\\/");
        if (pos != std::wstring::npos)
        {
            return path.substr(0, pos);
        }
        return {};
    }

    DWORD find_process_id(const std::wstring &process_name) noexcept
    {
        SmartHandle snapshot = make_smart_handle(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
        if (!is_valid_handle(snapshot.get()))
            return 0;

        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(PROCESSENTRY32W);

        if (!::Process32FirstW(static_cast<HANDLE>(snapshot.get()), &entry))
            return 0;

        do
        {
            if (process_name == entry.szExeFile)
                return entry.th32ProcessID;
        } while (::Process32NextW(static_cast<HANDLE>(snapshot.get()), &entry));

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

    bool inject_dll(DWORD process_id, const std::wstring &dll_path) noexcept
    {
        SmartHandle process = make_smart_handle(::OpenProcess(INJECTION_ACCESS, FALSE, process_id));
        if (!is_valid_handle(process.get()))
            return false;

        const std::size_t buffer_size = (dll_path.size() + 1) * sizeof(wchar_t);

        LPVOID remote_memory = ::VirtualAllocEx(
            static_cast<HANDLE>(process.get()),
            nullptr,
            buffer_size,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE);
        if (!remote_memory)
            return false;

        auto free_remote = [process_h = static_cast<HANDLE>(process.get()), remote_memory]() noexcept
        {
            ::VirtualFreeEx(process_h, remote_memory, 0, MEM_RELEASE);
        };

        if (!::WriteProcessMemory(
                static_cast<HANDLE>(process.get()),
                remote_memory,
                dll_path.c_str(),
                buffer_size,
                nullptr))
        {
            free_remote();
            return false;
        }

        HMODULE kernel32 = ::GetModuleHandleW(L"kernel32.dll");
        if (!kernel32)
        {
            free_remote();
            return false;
        }

        auto load_library_w = reinterpret_cast<LPTHREAD_START_ROUTINE>(
            ::GetProcAddress(kernel32, "LoadLibraryW"));
        if (!load_library_w)
        {
            free_remote();
            return false;
        }

        SmartHandle thread = make_smart_handle(::CreateRemoteThread(
            static_cast<HANDLE>(process.get()),
            nullptr,
            0,
            load_library_w,
            remote_memory,
            0,
            nullptr));
        if (!is_valid_handle(thread.get()))
        {
            free_remote();
            return false;
        }

        DWORD wait_result = ::WaitForSingleObject(static_cast<HANDLE>(thread.get()), INFINITE);
        if (wait_result != WAIT_OBJECT_0)
        {
            free_remote();
            return false;
        }

        DWORD exit_code = 0;
        if (!::GetExitCodeThread(static_cast<HANDLE>(thread.get()), &exit_code))
        {
            free_remote();
            return false;
        }

        free_remote();
        return exit_code != 0;
    }

    std::wstring resolve_absolute_path(const std::wstring &relative_path)
    {
        wchar_t absolute_buffer[MAX_PATH] = {};
        const DWORD length = ::GetFullPathNameW(relative_path.c_str(), MAX_PATH, absolute_buffer, nullptr);
        if (length == 0 || length >= MAX_PATH)
            return {};
        return std::wstring(absolute_buffer, length);
    }

    int run_injector(int argc, char *argv[])
    {
        if (sizeof(void *) != 8)
        {
            std::cerr << "Error: Injector must be compiled as x64 to inject into CS2 (x64).\n";
            return 1;
        }

        std::wstring process_name = L"cs2.exe";
        std::wstring dll_path = L"VorInternal.dll";

        if (argc >= 2)
        {
            process_name = utf8_to_wide(argv[1]);
        }
        if (argc >= 3)
        {
            dll_path = utf8_to_wide(argv[2]);
        }

        if (process_name.empty() || dll_path.empty())
        {
            std::cerr << "Invalid arguments\n";
            return 1;
        }

        std::wstring absolute_dll_path = resolve_absolute_path(dll_path);
        if (absolute_dll_path.empty() || !file_exists(absolute_dll_path))
        {
            if (absolute_dll_path.empty())
            {
                absolute_dll_path = dll_path;
            }
            if (!file_exists(absolute_dll_path))
            {
                std::wstring exe_dir = get_exe_directory();
                if (!exe_dir.empty())
                {
                    std::wstring fallback_path = exe_dir + L"\\" + dll_path;
                    if (file_exists(fallback_path))
                    {
                        absolute_dll_path = fallback_path;
                    }
                }
            }
            if (absolute_dll_path.empty() || !file_exists(absolute_dll_path))
            {
                std::wcerr << L"DLL not found: " << dll_path << L"\n";
                return 1;
            }
        }

        std::wcout << L"Waiting for " << process_name << L"...\n";
        const DWORD process_id = wait_for_process(process_name);
        if (process_id == 0)
        {
            std::wcerr << L"Process not found: " << process_name << L"\n";
            return 1;
        }

        if (inject_dll(process_id, absolute_dll_path))
        {
            std::wcout << L"Injected successfully into PID " << process_id << L"\n";
            return 0;
        }

        std::cerr << "Injection failed (error: " << ::GetLastError() << ")\n";
        return 1;
    }
}

int main(int argc, char *argv[])
{
    return run_injector(argc, argv);
}
