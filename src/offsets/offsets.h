#pragma once
#include "../dump/offsets.hpp"
#include "../dump/client_dll.hpp"
#include "../dump/engine2_dll.hpp"

namespace game
{
    namespace offsets
    {
        constexpr auto dwLocalPlayerPawn = cs2_dumper::offsets::client_dll::dwLocalPlayerPawn;
        constexpr auto dwEntityList = cs2_dumper::offsets::client_dll::dwEntityList;
        constexpr auto dwViewMatrix = cs2_dumper::offsets::client_dll::dwViewMatrix;
    }
    namespace fields
    {
        constexpr auto m_hPlayerPawn = cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn;
        constexpr auto m_bPawnIsAlive = cs2_dumper::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive;
        constexpr auto m_iszPlayerName = cs2_dumper::schemas::client_dll::CBasePlayerController::m_iszPlayerName;
        constexpr auto m_iHealth = cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth;
        constexpr auto m_iTeamNum = cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum;
        constexpr auto m_vOldOrigin = cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin;
        constexpr auto m_ArmorValue = cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_ArmorValue;
        constexpr auto m_bIsScoped = cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_bIsScoped;
        constexpr auto m_flFlashDuration = cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_flFlashDuration;
        constexpr auto m_angEyeAngles = cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_angEyeAngles;
        constexpr auto m_vecAbsVelocity = cs2_dumper::schemas::client_dll::C_BaseEntity::m_vecAbsVelocity;
        constexpr auto m_fFlags = cs2_dumper::schemas::client_dll::C_BaseEntity::m_fFlags;
        constexpr auto m_pItemServices = cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pItemServices;
        constexpr auto m_bHasDefuser = cs2_dumper::schemas::client_dll::CCSPlayer_ItemServices::m_bHasDefuser;
    }
}
