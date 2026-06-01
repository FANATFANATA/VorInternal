#include "cheat.h"
#include "../offsets/offsets.h"
#include <thread>
#include <chrono>
#include <cmath>
#include <Windows.h>

namespace consts
{
    constexpr int MAX_PLAYERS = 64;
    constexpr std::uint32_t HANDLE_MASK = 0x7FFF;
    constexpr std::uint32_t INVALID_HANDLE = 0xFFFFFFFF;
    constexpr int ENTITY_LIST_STRIDE = 112;
    constexpr int ENTITY_LIST_PAGE_SHIFT = 9;
    constexpr int ENTITY_LIST_PAGE_MASK = 0x1FF;
    constexpr int MAX_NAME_LENGTH = 128;
    constexpr int ENTITY_LIST_POINTER_SIZE = 8;
    constexpr int ENTITY_LIST_PAGE_OFFSET = 16;
    constexpr int FRAME_DELAY_MS = 1;
    constexpr std::uint32_t FL_ONGROUND = 0x1;
    constexpr std::uint32_t FL_DUCKING = 0x2;
    constexpr float DISTANCE_SCALE = 0.0254f;
}

inline HANDLE g_hCurrentProcess = GetCurrentProcess();

template <typename T>
inline bool read_mem(std::uintptr_t address, T &out)
{
    if (address < 0x10000)
        return false;
    SIZE_T bytes_read = 0;
    return ReadProcessMemory(g_hCurrentProcess, reinterpret_cast<LPCVOID>(address), &out, sizeof(T), &bytes_read) && bytes_read == sizeof(T);
}

inline bool read_string_safe(std::uintptr_t address, char *buffer, std::size_t size)
{
    if (address < 0x10000 || !buffer || size == 0)
        return false;
    SIZE_T bytes_read = 0;
    if (ReadProcessMemory(g_hCurrentProcess, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes_read))
    {
        buffer[size - 1] = '\0';
        return true;
    }
    return false;
}

std::uintptr_t get_ent(std::uintptr_t base, int i)
{
    std::uintptr_t list = 0;
    if (!read_mem(base + game::offsets::dwEntityList, list))
        return 0;

    std::uintptr_t entry = 0;
    if (!read_mem(list + (consts::ENTITY_LIST_POINTER_SIZE * (i >> consts::ENTITY_LIST_PAGE_SHIFT)) + consts::ENTITY_LIST_PAGE_OFFSET, entry))
        return 0;

    std::uintptr_t ent = 0;
    if (!read_mem(entry + consts::ENTITY_LIST_STRIDE * (i & consts::ENTITY_LIST_PAGE_MASK), ent))
        return 0;

    return ent;
}

std::uintptr_t get_pawn(std::uintptr_t base, std::uint32_t handle)
{
    if (handle == 0 || handle == consts::INVALID_HANDLE)
        return 0;
    return get_ent(base, static_cast<int>(handle & consts::HANDLE_MASK));
}

void cheat::worker(SharedState &s, const Config &cfg, std::atomic<bool> &is_running)
{
    std::uintptr_t client_base = 0;
    while (is_running.load() && !client_base)
    {
        client_base = reinterpret_cast<std::uintptr_t>(GetModuleHandleW(L"client.dll"));
        if (!client_base)
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::vector<PlayerData> tmp;
    tmp.reserve(consts::MAX_PLAYERS);

    while (is_running.load())
    {
        std::uintptr_t lp = 0;
        ViewMatrix vm{};

        if (!read_mem(client_base + game::offsets::dwLocalPlayerPawn, lp) ||
            !read_mem(client_base + game::offsets::dwViewMatrix, vm))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        std::uint8_t local_team = 0;
        Vector3 local_pos{};

        if (lp)
        {
            read_mem(lp + game::fields::m_iTeamNum, local_team);
            read_mem(lp + game::fields::m_vOldOrigin, local_pos);
        }

        tmp.clear();
        for (int i = 1; i <= consts::MAX_PLAYERS; ++i)
        {
            std::uintptr_t ctrl = get_ent(client_base, i);
            if (!ctrl)
                continue;

            std::uint32_t handle = 0;
            if (!read_mem(ctrl + game::fields::m_hPlayerPawn, handle))
                continue;

            std::uintptr_t p = get_pawn(client_base, handle);
            if (!p || p == lp)
                continue;

            bool alive = false;
            if (!read_mem(ctrl + game::fields::m_bPawnIsAlive, alive) || !alive)
                continue;

            PlayerData d;
            if (!read_mem(p + game::fields::m_iHealth, d.health) ||
                !read_mem(p + game::fields::m_iTeamNum, d.team) ||
                !read_mem(p + game::fields::m_vOldOrigin, d.position))
            {
                continue;
            }

            d.is_alive = true;
            d.distance = std::sqrt(std::pow(d.position.x - local_pos.x, 2) + std::pow(d.position.y - local_pos.y, 2) + std::pow(d.position.z - local_pos.z, 2)) * consts::DISTANCE_SCALE;

            read_mem(p + game::fields::m_ArmorValue, d.armor);
            read_mem(p + game::fields::m_bIsScoped, d.is_scoped);
            read_mem(p + game::fields::m_flFlashDuration, d.flash_duration);
            read_mem(p + game::fields::m_vecAbsVelocity, d.velocity);
            read_mem(p + game::fields::m_angEyeAngles, d.eye_angles);

            std::uint32_t flags = 0;
            if (read_mem(p + game::fields::m_fFlags, flags))
            {
                d.is_crouching = (flags & consts::FL_DUCKING) != 0;
                d.is_in_air = (flags & consts::FL_ONGROUND) == 0;
            }

            std::uintptr_t item_srv = 0;
            if (read_mem(p + game::fields::m_pItemServices, item_srv) && item_srv)
            {
                read_mem(item_srv + game::fields::m_bHasDefuser, d.has_defuser);
            }

            char buf[consts::MAX_NAME_LENGTH]{};
            if (read_string_safe(ctrl + game::fields::m_iszPlayerName, buf, consts::MAX_NAME_LENGTH))
            {
                d.name = buf;
            }
            tmp.push_back(std::move(d));
        }

        {
            std::lock_guard<std::mutex> lock(s.write_mutex);
            int w_idx = s.write_index.load();
            s.players_buffer[w_idx] = std::move(tmp);
            s.vm_buffer[w_idx] = vm;
            s.local_team_buffer[w_idx] = local_team;
            s.local_pos_buffer[w_idx] = local_pos;
            s.read_index.store(w_idx, std::memory_order_release);
            s.write_index.store(w_idx == 0 ? 1 : 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(consts::FRAME_DELAY_MS));
    }
}
