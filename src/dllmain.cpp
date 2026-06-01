#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <MinHook.h>
#include <cstdint>
#include <thread>
#include <atomic>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "types.h"
#include "math/math.h"
#include "config/config.h"
#include "cheat/cheat.h"
#include "render/render.h"

using Microsoft::WRL::ComPtr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::atomic<bool> g_bUnloadRequested{false};
HMODULE g_hModule = nullptr;

namespace
{
    constexpr UINT VTABLE_INDEX_PRESENT = 8;
    constexpr UINT BACK_BUFFER_INDEX = 0;

    std::atomic<bool> g_bInitialized{false};
    std::atomic<bool> g_bMenuOpen{false};
    std::atomic<bool> g_bShuttingDown{false};
    std::atomic<WNDPROC> g_oWndProc{nullptr};
    std::atomic<HWND> g_hwnd{nullptr};

    ComPtr<ID3D11Device> g_pDevice;
    ComPtr<ID3D11DeviceContext> g_pContext;
    ComPtr<ID3D11RenderTargetView> g_pRenderTargetView;
    ComPtr<ID3D11Texture2D> g_pBackBuffer;

    using PresentFn = HRESULT(__stdcall *)(IDXGISwapChain *, UINT, UINT);
    std::atomic<void *> g_oPresent{nullptr};

    SharedState g_sharedState;
    Config g_config;
    std::thread g_cheatThread;
    std::atomic<bool> g_running{true};

    LRESULT __stdcall hkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (uMsg == WM_KEYUP && wParam == VK_INSERT)
        {
            g_bMenuOpen.store(!g_bMenuOpen.load());
            return 0;
        }

        bool menuOpen = g_bMenuOpen.load();
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDrawCursor = menuOpen;

        if (menuOpen)
        {
            if (uMsg == WM_INPUT || ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
            {
                return 1;
            }
        }

        WNDPROC origProc = g_oWndProc.load();
        if (!origProc)
        {
            origProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
            if (origProc == hkWndProc)
                origProc = nullptr;
        }

        if (origProc)
        {
            return CallWindowProcW(origProc, hwnd, uMsg, wParam, lParam);
        }
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    void CleanupRenderTarget()
    {
        g_pRenderTargetView.Reset();
        g_pBackBuffer.Reset();
    }

    void CreateRenderTarget(IDXGISwapChain *pSwapChain)
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        if (SUCCEEDED(pSwapChain->GetBuffer(BACK_BUFFER_INDEX, __uuidof(ID3D11Texture2D), &pBackBuffer)))
        {
            ComPtr<ID3D11RenderTargetView> pRenderTargetView;
            if (SUCCEEDED(g_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pRenderTargetView)))
            {
                g_pRenderTargetView = pRenderTargetView;
                g_pBackBuffer = pBackBuffer;
            }
        }
    }

    DWORD WINAPI UnloadThread(LPVOID lpParam)
    {
        Sleep(200);

        if (g_bInitialized.load())
        {
            g_bShuttingDown.store(true);
            g_running.store(false);
            Sleep(200);

            if (g_cheatThread.joinable())
                g_cheatThread.join();
            config_manager::save("default", g_config);

            MH_DisableHook(MH_ALL_HOOKS);
            MH_Uninitialize();

            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            HWND hwnd = g_hwnd.load();
            if (hwnd)
            {
                WNDPROC origProc = g_oWndProc.load();
                if (!origProc)
                {
                    origProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
                    if (origProc == hkWndProc)
                        origProc = nullptr;
                }
                if (origProc)
                {
                    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(origProc));
                }
            }

            CleanupRenderTarget();
            g_pContext.Reset();
            g_pDevice.Reset();
            g_bInitialized.store(false);
        }

        FreeLibraryAndExitThread(g_hModule, 0);
        return 0;
    }

    HRESULT __stdcall hkPresent(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags)
    {
        auto origPresent = reinterpret_cast<PresentFn>(g_oPresent.load());
        if (!origPresent)
            return S_OK;

        if (g_bUnloadRequested.load())
        {
            HANDLE hThread = CreateThread(nullptr, 0, UnloadThread, nullptr, 0, nullptr);
            if (hThread)
                CloseHandle(hThread);
            return origPresent(pSwapChain, SyncInterval, Flags);
        }

        if (g_bShuttingDown.load())
        {
            return origPresent(pSwapChain, SyncInterval, Flags);
        }

        if (!g_bInitialized.load())
        {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), &g_pDevice)))
            {
                g_pDevice->GetImmediateContext(&g_pContext);

                DXGI_SWAP_CHAIN_DESC desc = {};
                pSwapChain->GetDesc(&desc);
                HWND hwnd = desc.OutputWindow;
                g_hwnd.store(hwnd);

                ImGui::CreateContext();
                ImGuiIO &io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                ImGui::StyleColorsDark();
                render::setup_monochrome();

                ImGui_ImplWin32_Init(hwnd);
                ImGui_ImplDX11_Init(g_pDevice.Get(), g_pContext.Get());

                g_oWndProc.store(reinterpret_cast<WNDPROC>(SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hkWndProc))));

                CreateRenderTarget(pSwapChain);
                g_bInitialized.store(true);
            }
        }

        if (g_bInitialized.load())
        {
            ComPtr<ID3D11Texture2D> pCurrentBackBuffer;
            if (SUCCEEDED(pSwapChain->GetBuffer(BACK_BUFFER_INDEX, __uuidof(ID3D11Texture2D), &pCurrentBackBuffer)))
            {
                if (pCurrentBackBuffer.Get() != g_pBackBuffer.Get())
                {
                    CleanupRenderTarget();
                    ComPtr<ID3D11RenderTargetView> pRenderTargetView;
                    if (SUCCEEDED(g_pDevice->CreateRenderTargetView(pCurrentBackBuffer.Get(), nullptr, &pRenderTargetView)))
                    {
                        g_pRenderTargetView = pRenderTargetView;
                        g_pBackBuffer = pCurrentBackBuffer;
                    }
                }
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            bool bMenuOpen = g_bMenuOpen.load();
            static char cfg_input[64] = "default";
            static std::string cfg_name = "default";

            int sw = static_cast<int>(ImGui::GetIO().DisplaySize.x);
            int sh = static_cast<int>(ImGui::GetIO().DisplaySize.y);

            int r_idx = g_sharedState.read_index.load(std::memory_order_acquire);
            auto src_players = g_sharedState.players_buffer[r_idx];
            auto vm = g_sharedState.vm_buffer[r_idx];
            auto local_team = g_sharedState.local_team_buffer[r_idx];

            std::vector<PlayerData> render_players;
            render_players.reserve(src_players.size());
            for (const auto &p : src_players)
            {
                if (!g_config.esp_teammates && p.team == local_team)
                    continue;
                PlayerData rp = p;
                bool feet_w2s = math::w2s(rp.position, rp.feet_screen, vm, sw, sh);
                bool head_w2s = math::w2s({rp.position.x, rp.position.y, rp.position.z + consts::HEAD_HEIGHT_OFFSET}, rp.head_screen, vm, sw, sh);
                rp.is_on_screen = feet_w2s && head_w2s;
                render_players.push_back(std::move(rp));
            }

            if (g_config.show_watermark)
            {
                render::draw_watermark(ImGui::GetForegroundDrawList());
            }
            if (bMenuOpen)
            {
                render::draw_menu(g_config, bMenuOpen, cfg_input, cfg_name);
            }
            if (g_config.esp_enabled)
            {
                render::draw_esp(ImGui::GetBackgroundDrawList(), render_players, g_config, vm, sw, sh);
            }
            g_bMenuOpen.store(bMenuOpen);

            ImGui::EndFrame();
            ImGui::Render();

            if (g_pRenderTargetView)
            {
                ID3D11RenderTargetView *rtv = g_pRenderTargetView.Get();
                g_pContext->OMSetRenderTargets(1, &rtv, nullptr);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            }
        }

        return origPresent(pSwapChain, SyncInterval, Flags);
    }

    void **GetDummySwapChainVTable()
    {
        WNDCLASSEXW wc = {sizeof(WNDCLASSEXW), CS_CLASSDC, DefWindowProcW, 0, 0, GetModuleHandleW(nullptr), nullptr, nullptr, nullptr, nullptr, L"DummyClass", nullptr};
        if (!RegisterClassExW(&wc))
            return nullptr;

        HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
        if (!hwnd)
        {
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return nullptr;
        }

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        ComPtr<ID3D11Device> pDevice;
        ComPtr<ID3D11DeviceContext> pContext;
        ComPtr<IDXGISwapChain> pSwapChain;
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

        if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, &featureLevel, &pContext)))
        {
            DestroyWindow(hwnd);
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return nullptr;
        }

        void **vtable = *reinterpret_cast<void ***>(pSwapChain.Get());

        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);

        return vtable;
    }

    DWORD WINAPI InitThread(LPVOID lpParam)
    {
        Sleep(1500);

        config_manager::load("default", g_config);
        g_cheatThread = std::thread(cheat::worker, std::ref(g_sharedState), std::ref(g_config), std::ref(g_running));

        if (MH_Initialize() != MH_OK)
            return 1;

        void **vtable = GetDummySwapChainVTable();
        if (!vtable)
            return 1;

        PresentFn pPresent = reinterpret_cast<PresentFn>(vtable[VTABLE_INDEX_PRESENT]);
        void *temp_oPresent = nullptr;
        if (MH_CreateHook(pPresent, &hkPresent, &temp_oPresent) != MH_OK)
            return 1;
        g_oPresent.store(temp_oPresent);

        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
            return 1;

        return 0;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, InitThread, hModule, 0, nullptr);
        if (hThread)
            CloseHandle(hThread);
    }
    return TRUE;
}
