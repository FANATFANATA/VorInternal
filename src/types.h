#pragma once
#include <string>
#include <cstdint>
#include <array>
#include <vector>
#include <mutex>
#include <atomic>
#include <cmath>

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct QAngle
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ViewMatrix
{
    float matrix[4][4]{};
};

struct PlayerData
{
    char name[128]{};
    int health = 0;
    int armor = 0;
    std::uint8_t team = 0;
    Vector3 position;
    Vector3 velocity;
    QAngle eye_angles;
    Vector3 feet_screen;
    Vector3 head_screen;
    float distance = 0.0f;
    bool is_alive = false;
    bool is_on_screen = false;
    bool has_defuser = false;
    bool is_scoped = false;
    bool is_crouching = false;
    bool is_in_air = false;
    float flash_duration = 0.0f;
};

struct Config
{
    std::uint32_t magic = 0x564F5201;
    std::uint32_t version = 9;
    bool esp_enabled = true;
    bool esp_teammates = false;
    bool esp_outline = true;
    float outline_thickness = 1.5f;
    bool show_boxes = true;
    int box_type = 0;
    float box_thickness = 1.5f;
    bool show_health = true;
    int hp_bar_pos = 0;
    float hp_bar_thickness = 2.5f;
    float hp_bar_offset = 4.0f;
    bool show_armor_bar = true;
    int armor_bar_pos = 1;
    float armor_bar_thickness = 2.5f;
    float armor_bar_offset = 4.0f;
    bool show_names = true;
    int name_pos = 0;
    bool show_snaplines = false;
    int snapline_type = 0;
    float snapline_thickness = 1.5f;
    bool show_armor = false;
    bool show_distance = true;
    bool show_defuse_kit = true;
    bool show_scoped = true;
    bool show_flash = true;
    bool show_offscreen_arrows = false;
    float text_size = 13.0f;
    float text_outline_thickness = 1.0f;
    float box_padding = 2.0f;
    float text_padding = 3.0f;
    bool show_watermark = true;
    float color_box_t[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float color_box_ct[4] = {0.0f, 0.5f, 1.0f, 1.0f};
    float color_snap_t[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float color_snap_ct[4] = {0.0f, 0.5f, 1.0f, 1.0f};
    float color_text[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float color_armor[4] = {0.2f, 0.6f, 1.0f, 1.0f};
    int language = 0;
};

struct SharedState
{
    std::array<std::vector<PlayerData>, 2> players_buffer;
    std::array<ViewMatrix, 2> vm_buffer;
    std::array<std::uint8_t, 2> local_team_buffer;
    std::array<Vector3, 2> local_pos_buffer;
    std::atomic<int> read_index{0};
    std::atomic<int> write_index{1};
    std::mutex write_mutex;
};

namespace consts
{
    constexpr float HEAD_HEIGHT_OFFSET = 72.0f;
}
