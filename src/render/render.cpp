#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "render.h"
#include "../config/config.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <unordered_map>
#include <cmath>
#include <atomic>

extern std::atomic<bool> g_bUnloadRequested;

namespace consts
{
    constexpr std::uint8_t TEAM_TERRORIST = 2;
    constexpr float BOX_WIDTH_DIVISOR = 2.0f;
    constexpr float CORNER_DIVISOR = 4.0f;
    constexpr float MAX_HEALTH_PERCENT = 1.0f;
    constexpr float MIN_HEALTH_PERCENT = 0.0f;
    constexpr int HEALTH_DIVISOR = 100;
    constexpr float MAX_ARMOR_VALUE = 100.0f;
    constexpr int BYTE_MAX = 255;
    constexpr float THICKNESS_SLIDER_MIN = 0.5f;
    constexpr float THICKNESS_SLIDER_MAX = 5.0f;
    constexpr float SIZE_SLIDER_MIN = 1.0f;
    constexpr float SIZE_SLIDER_MAX = 10.0f;
    constexpr float TEXT_SIZE_MIN = 8.0f;
    constexpr float TEXT_SIZE_MAX = 24.0f;
    constexpr int MAX_CONFIG_NAME_LENGTH = 64;
    constexpr int HP_BAR_BG_R = 40;
    constexpr int HP_BAR_BG_G = 40;
    constexpr int HP_BAR_BG_B = 40;
    constexpr int HP_BAR_BG_A = 200;
    constexpr ImU32 COL_BLACK = IM_COL32(0, 0, 0, 255);
    constexpr float UI_WINDOW_ROUNDING = 8.0f;
    constexpr float UI_ROUNDING = 4.0f;
    constexpr float UI_BORDER_SIZE = 1.0f;
    constexpr float UI_PAD = 15.0f;
    constexpr float UI_PAD_SMALL = 8.0f;
    constexpr float UI_SPACE = 10.0f;
    constexpr float UI_SPACE_SMALL = 5.0f;
    constexpr float UI_CHECK_SIZE = 16.0f;
    constexpr float UI_SLIDER_HEIGHT = 20.0f;
    constexpr float UI_SLIDER_WIDTH = 150.0f;
    constexpr float UI_BUTTON_HEIGHT = 24.0f;
    constexpr float UI_TAB_HEIGHT = 30.0f;
    constexpr float UI_SIDEBAR_WIDTH = 120.0f;
    constexpr float UI_CONTENT_WIDTH = 360.0f;
    constexpr float UI_WINDOW_W = 500.0f;
    constexpr float UI_WINDOW_H = 580.0f;
    constexpr float UI_FONT_SIZE = 16.0f;
    constexpr ImU32 UI_COL_BG = IM_COL32(20, 20, 20, 255);
    constexpr ImU32 UI_COL_CHILD = IM_COL32(25, 25, 25, 255);
    constexpr ImU32 UI_COL_FRAME = IM_COL32(30, 30, 30, 255);
    constexpr ImU32 UI_COL_BORDER = IM_COL32(60, 60, 60, 255);
    constexpr ImU32 UI_COL_ACCENT = IM_COL32(100, 150, 250, 255);
    constexpr ImU32 UI_COL_TEXT = IM_COL32(200, 200, 200, 255);
    constexpr ImU32 UI_COL_TEXT_DIM = IM_COL32(120, 120, 120, 255);
    constexpr ImU32 UI_COL_HOVER = IM_COL32(45, 45, 45, 255);
    constexpr ImU32 UI_COL_ACTIVE = IM_COL32(60, 60, 60, 255);
    constexpr float ANIM_SPEED_HOVER = 15.0f;
    constexpr float ANIM_SPEED_CHECK = 18.0f;
    constexpr float ANIM_SPEED_SLIDER = 20.0f;
    constexpr float ANIM_SPEED_TAB = 12.0f;
    constexpr float ANIM_SPEED_FADE = 8.0f;
    constexpr float ANIM_SPEED_SLIDE = 10.0f;
    constexpr float ANIM_SPEED_WATERMARK = 3.0f;
    constexpr float ANIM_SPEED_PULSE = 2.0f;
    constexpr float WATERMARK_PAD_X = 16.0f;
    constexpr float WATERMARK_PAD_Y = 10.0f;
    constexpr float WATERMARK_MARGIN = 20.0f;
    constexpr float WATERMARK_FONT_SIZE = 18.0f;
    constexpr float MAX_TEXT_WIDTH = 10000.0f;
    constexpr float TAB_INDICATOR_WIDTH = 3.0f;
    constexpr float CHECKBOX_INNER_PADDING = 6.0f;
    constexpr float ANIM_THRESHOLD = 0.001f;
    constexpr float PULSE_MIN = 0.85f;
    constexpr float PULSE_MAX = 1.0f;
    constexpr float OFFSCREEN_ARROW_MARGIN = 30.0f;
    constexpr float OFFSCREEN_ARROW_SIZE = 15.0f;
}

namespace tr
{
    inline const char *enable(int lang) { return lang == 1 ? "ВХ" : "ESP"; }
    inline const char *teammates(int lang) { return lang == 1 ? "Тиммейты" : "Teammates"; }
    inline const char *outline(int lang) { return lang == 1 ? "Обводка" : "Outline"; }
    inline const char *outline_thick(int lang) { return lang == 1 ? "Толщина обводки" : "Outline Thick"; }
    inline const char *boxes(int lang) { return lang == 1 ? "Боксы" : "Boxes"; }
    inline const char *box_thick(int lang) { return lang == 1 ? "Толщина боксов" : "Box Thick"; }
    inline const char *hp_bar(int lang) { return lang == 1 ? "ХП" : "HP Bar"; }
    inline const char *hp_bar_thick(int lang) { return lang == 1 ? "Толщина ХП" : "HP Bar Thick"; }
    inline const char *hp_bar_offset(int lang) { return lang == 1 ? "Отступ ХП" : "HP Bar Offset"; }
    inline const char *armor_bar(int lang) { return lang == 1 ? "Броня" : "Armor Bar"; }
    inline const char *armor_bar_thick(int lang) { return lang == 1 ? "Толщина брони" : "Armor Bar Thick"; }
    inline const char *armor_bar_offset(int lang) { return lang == 1 ? "Отступ брони" : "Armor Bar Offset"; }
    inline const char *names(int lang) { return lang == 1 ? "Ники" : "Names"; }
    inline const char *snaplines(int lang) { return lang == 1 ? "Линии" : "Snaplines"; }
    inline const char *snap_thick(int lang) { return lang == 1 ? "Толщина линий" : "Snapline Thick"; }
    inline const char *frame(int lang) { return lang == 1 ? "Рамка" : "Frame"; }
    inline const char *corners(int lang) { return lang == 1 ? "Углы" : "Corners"; }
    inline const char *left(int lang) { return lang == 1 ? "Слева" : "Left"; }
    inline const char *right(int lang) { return lang == 1 ? "Справа" : "Right"; }
    inline const char *top(int lang) { return lang == 1 ? "Сверху" : "Top"; }
    inline const char *bottom(int lang) { return lang == 1 ? "Снизу" : "Bottom"; }
    inline const char *center(int lang) { return lang == 1 ? "Центр" : "Center"; }
    inline const char *t_team(int lang) { return lang == 1 ? "Т" : "T"; }
    inline const char *ct_team(int lang) { return lang == 1 ? "КТ" : "CT"; }
    inline const char *text(int lang) { return lang == 1 ? "Текст" : "Text"; }
    inline const char *options(int lang) { return lang == 1 ? "Настройки" : "Options"; }
    inline const char *configs(int lang) { return lang == 1 ? "Конфиги" : "Configs"; }
    inline const char *visuals(int lang) { return lang == 1 ? "Визуалы" : "Visuals"; }
    inline const char *save(int lang) { return lang == 1 ? "Сохранить" : "Save"; }
    inline const char *load(int lang) { return lang == 1 ? "Загрузить" : "Load"; }
    inline const char *del(int lang) { return lang == 1 ? "Удалить" : "Delete"; }
    inline const char *saved_configs(int lang) { return lang == 1 ? "Сохраненные:" : "Saved:"; }
    inline const char *language(int lang) { return lang == 1 ? "Язык" : "Language"; }
    inline const char *armor(int lang) { return lang == 1 ? "Броня (Текст)" : "Armor (Text)"; }
    inline const char *distance(int lang) { return lang == 1 ? "Дистанция" : "Distance"; }
    inline const char *defuse_kit(int lang) { return lang == 1 ? "Дефьюз" : "Defuser"; }
    inline const char *scoped(int lang) { return lang == 1 ? "В скопе" : "Scoped"; }
    inline const char *flash(int lang) { return lang == 1 ? "Ослеплен" : "Flashed"; }
    inline const char *offscreen(int lang) { return lang == 1 ? "Стрелки вне фова" : "Offscreen lines"; }
    inline const char *advanced(int lang) { return lang == 1 ? "Прочее" : "Misc"; }
    inline const char *watermark(int lang) { return lang == 1 ? "Ватермарка" : "Watermark"; }
    inline const char *unload(int lang) { return lang == 1 ? "Выгрузить" : "Unload"; }
    inline const char *text_size(int lang) { return lang == 1 ? "Размер текста" : "Text Size"; }
    inline const char *text_outline(int lang) { return lang == 1 ? "Обводка текста" : "Text Outline"; }
    inline const char *box_padding(int lang) { return lang == 1 ? "Отступ боксов" : "Box Padding"; }
    inline const char *text_padding(int lang) { return lang == 1 ? "Отступ текста" : "Text Padding"; }

    inline const char *col_box_t(int lang) { return lang == 1 ? "Боксы Т" : "Boxes T"; }
    inline const char *col_box_ct(int lang) { return lang == 1 ? "Боксы КТ" : "Boxes CT"; }
    inline const char *col_snap_t(int lang) { return lang == 1 ? "Линии Т" : "Lines T"; }
    inline const char *col_snap_ct(int lang) { return lang == 1 ? "Линии КТ" : "Lines CT"; }
    inline const char *col_text(int lang) { return lang == 1 ? "Цвет текста" : "Text Color"; }
    inline const char *col_armor(int lang) { return lang == 1 ? "Цвет брони" : "Armor Color"; }
}

namespace
{
    inline float ease_out_cubic(float t)
    {
        float t1 = 1.0f - t;
        return 1.0f - t1 * t1 * t1;
    }

    inline ImVec4 u32_to_vec4(ImU32 c)
    {
        return ImVec4(
            ((c >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
            ((c >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
            ((c >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f,
            ((c >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f);
    }

    inline ImU32 lerp_color(ImU32 c1, ImU32 c2, float t)
    {
        if (t < 0.0f)
            t = 0.0f;
        if (t > 1.0f)
            t = 1.0f;
        ImVec4 v1 = u32_to_vec4(c1);
        ImVec4 v2 = u32_to_vec4(c2);
        return ImGui::ColorConvertFloat4ToU32(ImVec4(
            v1.x + (v2.x - v1.x) * t,
            v1.y + (v2.y - v1.y) * t,
            v1.z + (v2.z - v1.z) * t,
            v1.w + (v2.w - v1.w) * t));
    }

    inline ImU32 apply_alpha(ImU32 c, float a)
    {
        ImVec4 v = u32_to_vec4(c);
        v.w *= a;
        if (v.w < 0.0f)
            v.w = 0.0f;
        if (v.w > 1.0f)
            v.w = 1.0f;
        return ImGui::ColorConvertFloat4ToU32(v);
    }

    inline bool w2s_unclamped(const Vector3 &world, Vector3 &screen, const ViewMatrix &vm, int w, int h)
    {
        float clip_w = vm.matrix[3][0] * world.x + vm.matrix[3][1] * world.y + vm.matrix[3][2] * world.z + vm.matrix[3][3];
        if (std::abs(clip_w) < 0.001f)
            return false;
        float x = vm.matrix[0][0] * world.x + vm.matrix[0][1] * world.y + vm.matrix[0][2] * world.z + vm.matrix[0][3];
        float y = vm.matrix[1][0] * world.x + vm.matrix[1][1] * world.y + vm.matrix[1][2] * world.z + vm.matrix[1][3];
        screen.x = (static_cast<float>(w) / 2.0f) + (x / clip_w * (static_cast<float>(w) / 2.0f));
        screen.y = (static_cast<float>(h) / 2.0f) - (y / clip_w * (static_cast<float>(h) / 2.0f));
        return true;
    }

    namespace anim
    {
        static std::unordered_map<ImGuiID, float> states;
        float update(ImGuiID id, float target, float speed = 10.0f)
        {
            float &val = states[id];
            float dt = ImGui::GetIO().DeltaTime;
            float diff = target - val;
            if (std::abs(diff) < consts::ANIM_THRESHOLD)
            {
                val = target;
                return val;
            }
            float eased_speed = speed * (1.0f + std::abs(diff) * 2.0f);
            val += diff * dt * eased_speed;
            return val;
        }
        float update_eased(ImGuiID id, float target, float speed = 10.0f)
        {
            float &val = states[id];
            float dt = ImGui::GetIO().DeltaTime;
            float diff = target - val;
            if (std::abs(diff) < consts::ANIM_THRESHOLD)
            {
                val = target;
                return val;
            }
            float step = diff * dt * speed;
            float eased_step = step * ease_out_cubic(std::abs(diff));
            val += eased_step;
            return val;
        }
    }

    namespace ui
    {
        void text(const char *label)
        {
            ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.00f), "%s", label);
        }

        bool checkbox(const char *label, bool *v)
        {
            ImGui::PushID(label);
            bool changed = false;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float size = consts::UI_CHECK_SIZE;
            float label_w = ImGui::CalcTextSize(label).x;
            if (ImGui::InvisibleButton("##cb", ImVec2(size + consts::UI_SPACE_SMALL + label_w, size)))
            {
                *v = !(*v);
                changed = true;
            }
            bool hovered = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            ImVec2 c_min = pos;
            ImVec2 c_max = ImVec2(pos.x + size, pos.y + size);
            float rounding = consts::UI_ROUNDING;
            ImGuiID id = ImGui::GetID("##cb");
            float hover_anim = anim::update_eased(id + 1, hovered ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            float check_anim = anim::update_eased(id + 2, *v ? 1.0f : 0.0f, consts::ANIM_SPEED_CHECK);
            float pulse_time = static_cast<float>(ImGui::GetTime()) * consts::ANIM_SPEED_PULSE;
            float pulse = (*v && hovered) ? consts::PULSE_MIN + (consts::PULSE_MAX - consts::PULSE_MIN) * (std::sin(pulse_time) * 0.5f + 0.5f) : 1.0f;
            ImU32 border_col = lerp_color(consts::UI_COL_BORDER, consts::UI_COL_ACCENT, hover_anim);
            float scale = 1.0f + hover_anim * 0.1f;
            float scaled_size = size * scale * pulse;
            float offset = (size - scaled_size) / 2.0f;
            ImVec2 scaled_min = ImVec2(c_min.x + offset, c_min.y + offset);
            ImVec2 scaled_max = ImVec2(c_max.x - offset, c_max.y - offset);
            dl->AddRectFilled(scaled_min, scaled_max, consts::UI_COL_FRAME, rounding * scale);
            dl->AddRect(scaled_min, scaled_max, border_col, rounding * scale);
            if (check_anim > 0.01f)
            {
                float eased_check = ease_out_cubic(check_anim);
                float inner_size = (scaled_size - consts::CHECKBOX_INNER_PADDING) * eased_check;
                float inner_offset = (scaled_size - inner_size) / 2.0f;
                dl->AddRectFilled(
                    ImVec2(scaled_min.x + inner_offset, scaled_min.y + inner_offset),
                    ImVec2(scaled_max.x - inner_offset, scaled_max.y - inner_offset),
                    consts::UI_COL_ACCENT,
                    rounding * eased_check);
            }
            dl->AddText(ImVec2(c_max.x + consts::UI_SPACE_SMALL, pos.y), consts::UI_COL_TEXT, label);
            ImGui::PopID();
            return changed;
        }

        bool slider_float(const char *id, float *v, float min, float max)
        {
            ImGui::PushID(id);
            bool changed = false;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float width = consts::UI_SLIDER_WIDTH;
            float height = consts::UI_SLIDER_HEIGHT;
            ImGui::InvisibleButton("##sl", ImVec2(width, height));
            bool active = ImGui::IsItemActive();
            bool hovered = ImGui::IsItemHovered();
            if (hovered && ImGui::IsMouseClicked(0))
            {
                float click_x = ImGui::GetIO().MousePos.x - pos.x;
                float ratio = click_x / width;
                *v = min + ratio * (max - min);
                changed = true;
            }
            if (active && ImGui::IsMouseDragging(0))
            {
                float click_x = ImGui::GetIO().MousePos.x - pos.x;
                float ratio = click_x / width;
                *v = min + ratio * (max - min);
                changed = true;
            }
            if (*v < min)
                *v = min;
            if (*v > max)
                *v = max;
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float ratio = (*v - min) / (max - min);
            float rounding = consts::UI_ROUNDING;
            ImGuiID sid = ImGui::GetID("##sl");
            float hover_anim = anim::update_eased(sid + 1, hovered || active ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            float anim_ratio = anim::update(sid + 2, ratio, consts::ANIM_SPEED_SLIDER);
            float eased_ratio = ease_out_cubic(anim_ratio);
            ImU32 border_col = lerp_color(consts::UI_COL_BORDER, consts::UI_COL_ACCENT, hover_anim);
            float scale = 1.0f + hover_anim * 0.05f;
            float scaled_height = height * scale;
            float y_offset = (height - scaled_height) / 2.0f;
            ImVec2 scaled_min = ImVec2(pos.x, pos.y + y_offset);
            ImVec2 scaled_max = ImVec2(pos.x + width, pos.y + y_offset + scaled_height);
            dl->AddRectFilled(scaled_min, scaled_max, consts::UI_COL_FRAME, rounding * scale);
            dl->AddRectFilled(scaled_min, ImVec2(scaled_min.x + width * eased_ratio, scaled_max.y), consts::UI_COL_ACCENT, rounding * scale);
            dl->AddRect(scaled_min, scaled_max, border_col, rounding * scale);
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.1f", *v);
            ImVec2 val_sz = ImGui::CalcTextSize(buf);
            dl->AddText(ImVec2(scaled_min.x + width - val_sz.x - consts::UI_PAD_SMALL, scaled_min.y + (scaled_height - val_sz.y) / 2.0f), consts::UI_COL_TEXT, buf);
            ImGui::PopID();
            return changed;
        }

        bool combo(const char *id, int *current, const char *const items[], int count)
        {
            ImGui::PushID(id);
            bool changed = false;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float width = consts::UI_SLIDER_WIDTH;
            float height = consts::UI_BUTTON_HEIGHT;
            if (ImGui::InvisibleButton("##combo", ImVec2(width, height)))
            {
                ImGui::OpenPopup(id);
            }
            bool hovered = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float rounding = consts::UI_ROUNDING;
            ImGuiID cid = ImGui::GetID("##combo");
            float hover_anim = anim::update_eased(cid + 1, hovered ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            ImU32 border_col = lerp_color(consts::UI_COL_BORDER, consts::UI_COL_ACCENT, hover_anim);
            float scale = 1.0f + hover_anim * 0.05f;
            float scaled_height = height * scale;
            float y_offset = (height - scaled_height) / 2.0f;
            ImVec2 scaled_min = ImVec2(pos.x, pos.y + y_offset);
            ImVec2 scaled_max = ImVec2(pos.x + width, pos.y + y_offset + scaled_height);
            dl->AddRectFilled(scaled_min, scaled_max, consts::UI_COL_FRAME, rounding * scale);
            dl->AddRect(scaled_min, scaled_max, border_col, rounding * scale);
            ImVec2 txt_sz = ImGui::CalcTextSize(items[*current]);
            dl->AddText(ImVec2(scaled_min.x + consts::UI_PAD_SMALL, scaled_min.y + (scaled_height - txt_sz.y) / 2.0f), consts::UI_COL_TEXT, items[*current]);
            if (ImGui::BeginPopup(id))
            {
                for (int i = 0; i < count; i++)
                {
                    if (ImGui::Selectable(items[i], i == *current))
                    {
                        *current = i;
                        changed = true;
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
            return changed;
        }

        bool button(const char *label, float width)
        {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImVec2(width, consts::UI_BUTTON_HEIGHT);
            if (width == 0.0f)
                size.x = ImGui::CalcTextSize(label).x + consts::UI_PAD * 2.0f;
            if (ImGui::InvisibleButton(label, size))
                return true;
            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float rounding = consts::UI_ROUNDING;
            ImGuiID bid = ImGui::GetID(label);
            float hover_anim = anim::update_eased(bid + 1, hovered ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            float active_anim = anim::update_eased(bid + 2, active ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            ImU32 bg = lerp_color(consts::UI_COL_FRAME, consts::UI_COL_HOVER, hover_anim);
            bg = lerp_color(bg, consts::UI_COL_ACCENT, active_anim);
            ImU32 border_col = lerp_color(consts::UI_COL_BORDER, consts::UI_COL_ACCENT, hover_anim + active_anim);
            float scale = 1.0f + hover_anim * 0.08f - active_anim * 0.05f;
            float scaled_w = size.x * scale;
            float scaled_h = size.y * scale;
            float x_offset = (size.x - scaled_w) / 2.0f;
            float y_offset = (size.y - scaled_h) / 2.0f;
            ImVec2 scaled_min = ImVec2(pos.x + x_offset, pos.y + y_offset);
            ImVec2 scaled_max = ImVec2(pos.x + x_offset + scaled_w, pos.y + y_offset + scaled_h);
            dl->AddRectFilled(scaled_min, scaled_max, bg, rounding * scale);
            dl->AddRect(scaled_min, scaled_max, border_col, rounding * scale);
            ImVec2 text_size = ImGui::CalcTextSize(label);
            dl->AddText(ImVec2(scaled_min.x + (scaled_w - text_size.x) / 2.0f, scaled_min.y + (scaled_h - text_size.y) / 2.0f), consts::UI_COL_TEXT, label);
            return false;
        }

        bool tab(const char *label, bool active)
        {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, consts::UI_TAB_HEIGHT);
            if (ImGui::InvisibleButton(label, size))
                return true;
            bool hovered = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float rounding = consts::UI_ROUNDING;
            ImGuiID tid = ImGui::GetID(label);
            float hover_anim = anim::update_eased(tid + 1, hovered ? 1.0f : 0.0f, consts::ANIM_SPEED_HOVER);
            float active_anim = anim::update_eased(tid + 2, active ? 1.0f : 0.0f, consts::ANIM_SPEED_TAB);
            ImU32 bg = lerp_color(consts::UI_COL_FRAME, consts::UI_COL_HOVER, hover_anim);
            bg = lerp_color(bg, consts::UI_COL_ACTIVE, active_anim);
            float scale = 1.0f + hover_anim * 0.03f;
            float scaled_w = size.x * scale;
            float x_offset = (size.x - scaled_w) / 2.0f;
            ImVec2 scaled_min = ImVec2(pos.x + x_offset, pos.y);
            ImVec2 scaled_max = ImVec2(pos.x + x_offset + scaled_w, pos.y + size.y);
            dl->AddRectFilled(scaled_min, scaled_max, bg, rounding * scale);
            if (active_anim > 0.01f)
            {
                float eased_active = ease_out_cubic(active_anim);
                float ind_w = consts::TAB_INDICATOR_WIDTH * eased_active;
                dl->AddRectFilled(scaled_min, ImVec2(scaled_min.x + ind_w, scaled_max.y), consts::UI_COL_ACCENT, 0.0f);
            }
            ImVec2 text_size = ImGui::CalcTextSize(label);
            ImU32 text_col = lerp_color(consts::UI_COL_TEXT_DIM, consts::UI_COL_TEXT, active_anim);
            dl->AddText(ImVec2(scaled_min.x + consts::UI_PAD, scaled_min.y + (size.y - text_size.y) / 2.0f), text_col, label);
            return false;
        }

        bool input_text(const char *id, char *buf, std::size_t size)
        {
            ImGui::PushID(id);
            bool changed = ImGui::InputText("##input", buf, size);
            ImGui::PopID();
            return changed;
        }

        bool color_edit(const char *label, float col[4])
        {
            ImGui::PushID(label);
            text(label);
            bool changed = ImGui::ColorEdit4("##color", col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::PopID();
            return changed;
        }

        bool selectable(const char *label, bool selected)
        {
            return ImGui::Selectable(label, selected);
        }
    }

    void draw_snapline(ImDrawList *const dl, const ImVec4 &col, const Vector3 &feet, const Config &cfg, int sw, int sh)
    {
        ImVec2 start{};
        if (cfg.snapline_type == 0)
            start = ImVec2(static_cast<float>(sw) / consts::BOX_WIDTH_DIVISOR, static_cast<float>(sh));
        else if (cfg.snapline_type == 1)
            start = ImVec2(static_cast<float>(sw) / consts::BOX_WIDTH_DIVISOR, static_cast<float>(sh) / consts::BOX_WIDTH_DIVISOR);
        else
            start = ImVec2(static_cast<float>(sw) / consts::BOX_WIDTH_DIVISOR, 0.0f);
        const ImVec2 end(feet.x, feet.y);
        const ImU32 clr = ImGui::ColorConvertFloat4ToU32(col);
        if (cfg.esp_outline)
            dl->AddLine(start, end, consts::COL_BLACK, cfg.snapline_thickness + cfg.outline_thickness);
        dl->AddLine(start, end, clr, cfg.snapline_thickness);
    }

    void draw_box(ImDrawList *const dl, const ImVec4 &col, float l, float t, float r, float b, float w, float h, const Config &cfg)
    {
        const ImU32 clr = ImGui::ColorConvertFloat4ToU32(col);
        const float padding = cfg.box_padding;
        if (cfg.box_type == 0)
        {
            if (cfg.esp_outline)
                dl->AddRect(ImVec2(l - padding, t - padding), ImVec2(r + padding, b + padding), consts::COL_BLACK, 0.0f, 0, cfg.box_thickness + cfg.outline_thickness);
            dl->AddRect(ImVec2(l, t), ImVec2(r, b), clr, 0.0f, 0, cfg.box_thickness);
        }
        else
        {
            auto draw_c = [&](ImU32 c, float th)
            {
                const float lw = w / consts::CORNER_DIVISOR;
                const float lh = h / consts::CORNER_DIVISOR;
                dl->AddLine(ImVec2(l, t), ImVec2(l + lw, t), c, th);
                dl->AddLine(ImVec2(l, t), ImVec2(l, t + lh), c, th);
                dl->AddLine(ImVec2(r, t), ImVec2(r - lw, t), c, th);
                dl->AddLine(ImVec2(r, t), ImVec2(r, t + lh), c, th);
                dl->AddLine(ImVec2(l, b), ImVec2(l + lw, b), c, th);
                dl->AddLine(ImVec2(l, b), ImVec2(l, b - lh), c, th);
                dl->AddLine(ImVec2(r, b), ImVec2(r - lw, b), c, th);
                dl->AddLine(ImVec2(r, b), ImVec2(r, b - lh), c, th);
            };
            if (cfg.esp_outline)
                draw_c(consts::COL_BLACK, cfg.box_thickness + cfg.outline_thickness);
            draw_c(clr, cfg.box_thickness);
        }
    }

    void draw_health(ImDrawList *const dl, int health, float l, float t, float r, float b, float w, float h, const Config &cfg)
    {
        const float pct = std::clamp(static_cast<float>(health) / static_cast<float>(consts::HEALTH_DIVISOR), consts::MIN_HEALTH_PERCENT, consts::MAX_HEALTH_PERCENT);
        const ImU32 hclr = IM_COL32(static_cast<int>((consts::MAX_HEALTH_PERCENT - pct) * consts::BYTE_MAX), static_cast<int>(pct * consts::BYTE_MAX), 0, consts::BYTE_MAX);
        const float thickness = cfg.hp_bar_thickness;
        const float offset = cfg.hp_bar_offset;
        const float padding = cfg.box_padding;
        float bl, bt, br, bb, fl, ft, fr, fb;
        if (cfg.hp_bar_pos == 0)
        {
            bl = l - offset;
            bt = t;
            br = l - offset + thickness;
            bb = b;
            fl = bl;
            ft = b - (h * pct);
            fr = br;
            fb = b;
        }
        else if (cfg.hp_bar_pos == 1)
        {
            bl = r + offset - thickness;
            bt = t;
            br = r + offset;
            bb = b;
            fl = bl;
            ft = b - (h * pct);
            fr = br;
            fb = b;
        }
        else if (cfg.hp_bar_pos == 2)
        {
            bl = l;
            bt = t - offset;
            br = r;
            bb = t - offset + thickness;
            fl = l;
            ft = bt;
            fr = l + (w * pct);
            fb = bb;
        }
        else
        {
            bl = l;
            bt = b + offset - thickness;
            br = r;
            bb = b + offset;
            fl = l;
            ft = bt;
            fr = l + (w * pct);
            fb = bb;
        }
        if (cfg.esp_outline)
            dl->AddRectFilled(ImVec2(bl - padding, bt - padding), ImVec2(br + padding, bb + padding), consts::COL_BLACK);
        dl->AddRectFilled(ImVec2(bl, bt), ImVec2(br, bb), IM_COL32(consts::HP_BAR_BG_R, consts::HP_BAR_BG_G, consts::HP_BAR_BG_B, consts::HP_BAR_BG_A));
        dl->AddRectFilled(ImVec2(fl, ft), ImVec2(fr, fb), hclr);
    }

    void draw_armor(ImDrawList *const dl, int armor, float l, float t, float r, float b, float w, float h, const Config &cfg)
    {
        if (armor <= 0)
            return;
        const float pct = std::clamp(static_cast<float>(armor) / consts::MAX_ARMOR_VALUE, consts::MIN_HEALTH_PERCENT, consts::MAX_HEALTH_PERCENT);
        const ImU32 aclr = ImGui::ColorConvertFloat4ToU32(ImVec4(cfg.color_armor[0], cfg.color_armor[1], cfg.color_armor[2], cfg.color_armor[3]));
        const float thickness = cfg.armor_bar_thickness;
        const float offset = cfg.armor_bar_offset;
        const float padding = cfg.box_padding;
        float bl, bt, br, bb, fl, ft, fr, fb;
        if (cfg.armor_bar_pos == 0)
        {
            bl = l - offset;
            bt = t;
            br = l - offset + thickness;
            bb = b;
            fl = bl;
            ft = b - (h * pct);
            fr = br;
            fb = b;
        }
        else if (cfg.armor_bar_pos == 1)
        {
            bl = r + offset - thickness;
            bt = t;
            br = r + offset;
            bb = b;
            fl = bl;
            ft = b - (h * pct);
            fr = br;
            fb = b;
        }
        else if (cfg.armor_bar_pos == 2)
        {
            bl = l;
            bt = t - offset;
            br = r;
            bb = t - offset + thickness;
            fl = l;
            ft = bt;
            fr = l + (w * pct);
            fb = bb;
        }
        else
        {
            bl = l;
            bt = b + offset - thickness;
            br = r;
            bb = b + offset;
            fl = l;
            ft = bt;
            fr = l + (w * pct);
            fb = bb;
        }
        if (cfg.esp_outline)
            dl->AddRectFilled(ImVec2(bl - padding, bt - padding), ImVec2(br + padding, bb + padding), consts::COL_BLACK);
        dl->AddRectFilled(ImVec2(bl, bt), ImVec2(br, bb), IM_COL32(consts::HP_BAR_BG_R, consts::HP_BAR_BG_G, consts::HP_BAR_BG_B, consts::HP_BAR_BG_A));
        dl->AddRectFilled(ImVec2(fl, ft), ImVec2(fr, fb), aclr);
    }

    void draw_text_with_outline(ImDrawList *const dl, ImVec2 pos, ImU32 col, const char *text, float text_size, float outline_thick)
    {
        ImFont *font = ImGui::GetFont();
        if (outline_thick > 0.0f)
        {
            font->Scale = text_size / consts::UI_FONT_SIZE;
            dl->AddText(font, text_size, ImVec2(pos.x - outline_thick, pos.y), consts::COL_BLACK, text);
            dl->AddText(font, text_size, ImVec2(pos.x + outline_thick, pos.y), consts::COL_BLACK, text);
            dl->AddText(font, text_size, ImVec2(pos.x, pos.y - outline_thick), consts::COL_BLACK, text);
            dl->AddText(font, text_size, ImVec2(pos.x, pos.y + outline_thick), consts::COL_BLACK, text);
            font->Scale = 1.0f;
        }
        font->Scale = text_size / consts::UI_FONT_SIZE;
        dl->AddText(font, text_size, pos, col, text);
        font->Scale = 1.0f;
    }

    void draw_offscreen_arrow(ImDrawList *const dl, const Vector3 &screen_pos, const ImVec4 &col, int sw, int sh)
    {
        ImVec2 center(static_cast<float>(sw) / 2.0f, static_cast<float>(sh) / 2.0f);
        ImVec2 target(screen_pos.x, screen_pos.y);
        float dx = target.x - center.x;
        float dy = target.y - center.y;
        float angle = std::atan2(dy, dx);
        float margin = consts::OFFSCREEN_ARROW_MARGIN;
        float max_w = static_cast<float>(sw) - margin * 2.0f;
        float max_h = static_cast<float>(sh) - margin * 2.0f;
        float intersect_x = center.x;
        float intersect_y = center.y;
        float cos_a = std::cos(angle);
        float sin_a = std::sin(angle);
        if (std::abs(cos_a) * max_h > std::abs(sin_a) * max_w)
        {
            float sign = cos_a > 0 ? 1.0f : -1.0f;
            intersect_x = center.x + sign * max_w / 2.0f;
            intersect_y = center.y + (sin_a / cos_a) * sign * max_w / 2.0f;
        }
        else
        {
            float sign = sin_a > 0 ? 1.0f : -1.0f;
            intersect_y = center.y + sign * max_h / 2.0f;
            intersect_x = center.x + (cos_a / sin_a) * sign * max_h / 2.0f;
        }
        ImVec2 arrow_pos(intersect_x, intersect_y);
        float arrow_size = consts::OFFSCREEN_ARROW_SIZE;
        ImVec2 p1(arrow_pos.x + std::cos(angle) * arrow_size, arrow_pos.y + std::sin(angle) * arrow_size);
        ImVec2 p2(arrow_pos.x + std::cos(angle + 2.5f) * arrow_size, arrow_pos.y + std::sin(angle + 2.5f) * arrow_size);
        ImVec2 p3(arrow_pos.x + std::cos(angle - 2.5f) * arrow_size, arrow_pos.y + std::sin(angle - 2.5f) * arrow_size);
        ImU32 clr = ImGui::ColorConvertFloat4ToU32(col);
        dl->AddTriangleFilled(p1, p2, p3, clr);
        dl->AddTriangle(p1, p2, p3, consts::COL_BLACK, 2.0f);
    }
}

void render::setup_monochrome()
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", consts::UI_FONT_SIZE);
    ImGuiStyle &s = ImGui::GetStyle();
    s.WindowRounding = consts::UI_WINDOW_ROUNDING;
    s.ChildRounding = consts::UI_WINDOW_ROUNDING;
    s.FrameRounding = consts::UI_ROUNDING;
    s.PopupRounding = consts::UI_ROUNDING;
    s.GrabRounding = consts::UI_ROUNDING;
    s.TabRounding = consts::UI_ROUNDING;
    s.WindowBorderSize = consts::UI_BORDER_SIZE;
    s.FrameBorderSize = 0.0f;
    s.PopupBorderSize = consts::UI_BORDER_SIZE;
    s.WindowPadding = ImVec2(consts::UI_PAD, consts::UI_PAD);
    s.FramePadding = ImVec2(consts::UI_PAD_SMALL, consts::UI_PAD_SMALL);
    s.ItemSpacing = ImVec2(consts::UI_SPACE, consts::UI_SPACE);
    s.ItemInnerSpacing = ImVec2(consts::UI_SPACE_SMALL, consts::UI_SPACE_SMALL);
    ImVec4 *c = s.Colors;
    c[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    c[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    c[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    c[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    c[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    c[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    c[ImGuiCol_CheckMark] = ImVec4(0.39f, 0.58f, 0.98f, 1.00f);
    c[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.58f, 0.98f, 1.00f);
    c[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.58f, 0.98f, 1.00f);
    c[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    c[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
}

void render::draw_esp(ImDrawList *const dl, const std::vector<PlayerData> &players, const Config &cfg, const ViewMatrix &vm, int sw, int sh)
{
    if (!cfg.esp_enabled || players.empty())
        return;

    const ImVec4 col_box_t(cfg.color_box_t[0], cfg.color_box_t[1], cfg.color_box_t[2], cfg.color_box_t[3]);
    const ImVec4 col_box_ct(cfg.color_box_ct[0], cfg.color_box_ct[1], cfg.color_box_ct[2], cfg.color_box_ct[3]);
    const ImVec4 col_snap_t(cfg.color_snap_t[0], cfg.color_snap_t[1], cfg.color_snap_t[2], cfg.color_snap_t[3]);
    const ImVec4 col_snap_ct(cfg.color_snap_ct[0], cfg.color_snap_ct[1], cfg.color_snap_ct[2], cfg.color_snap_ct[3]);
    const ImVec4 col_txt(cfg.color_text[0], cfg.color_text[1], cfg.color_text[2], cfg.color_text[3]);

    for (const auto &p : players)
    {
        bool is_t = (p.team == consts::TEAM_TERRORIST);
        const ImVec4 &box_col = is_t ? col_box_t : col_box_ct;
        const ImVec4 &snap_col = is_t ? col_snap_t : col_snap_ct;

        if (!p.is_on_screen)
        {
            if (cfg.show_offscreen_arrows)
            {
                Vector3 offscreen_pos{};
                if (w2s_unclamped(p.position, offscreen_pos, vm, sw, sh))
                    draw_offscreen_arrow(dl, offscreen_pos, box_col, sw, sh);
            }
            continue;
        }
        const float h = p.feet_screen.y - p.head_screen.y;
        if (h <= 0.0f)
            continue;
        const float w = h / consts::BOX_WIDTH_DIVISOR;
        const float l = p.head_screen.x - w / consts::BOX_WIDTH_DIVISOR;
        const float t = p.head_screen.y;
        const float r = l + w;
        const float b = p.feet_screen.y;

        if (cfg.show_snaplines)
            draw_snapline(dl, snap_col, p.feet_screen, cfg, sw, sh);
        if (cfg.show_boxes)
            draw_box(dl, box_col, l, t, r, b, w, h, cfg);
        if (cfg.show_health)
            draw_health(dl, p.health, l, t, r, b, w, h, cfg);
        if (cfg.show_armor_bar)
            draw_armor(dl, p.armor, l, t, r, b, w, h, cfg);

        std::string info_text;
        if (cfg.show_names)
            info_text += p.name; info_text += "\n";
        if (cfg.show_distance)
            info_text += std::to_string(static_cast<int>(p.distance)) + "m\n";
        if (cfg.show_armor && p.armor > 0)
            info_text += "Armor: " + std::to_string(p.armor) + "\n";
        if (cfg.show_defuse_kit && p.has_defuser)
            info_text += "Defuser\n";
        if (cfg.show_scoped && p.is_scoped)
            info_text += "Scoped\n";
        if (cfg.show_flash && p.flash_duration > 0.0f)
            info_text += "Flashed\n";
        if (!info_text.empty())
        {
            info_text.pop_back();
            ImVec2 text_pos(r + cfg.text_padding, t);
            draw_text_with_outline(dl, text_pos, ImGui::ColorConvertFloat4ToU32(col_txt), info_text.c_str(), cfg.text_size, cfg.text_outline_thickness);
        }
    }
}

void render::draw_menu(Config &cfg, bool &menu, char *cfg_input, std::string &cfg_name)
{
    static int active_tab = 0;
    static int last_tab = -1;
    static float fade_alpha = 0.0f;
    static float slide_offset = 0.0f;
    static bool menu_visible = false;
    static float menu_alpha = 0.0f;
    if (menu && !menu_visible)
    {
        menu_visible = true;
    }
    else if (!menu && menu_visible)
    {
        menu_alpha -= ImGui::GetIO().DeltaTime * consts::ANIM_SPEED_FADE;
        if (menu_alpha <= 0.0f)
        {
            menu_alpha = 0.0f;
            menu_visible = false;
        }
    }
    if (menu_visible)
    {
        menu_alpha += ImGui::GetIO().DeltaTime * consts::ANIM_SPEED_FADE;
        if (menu_alpha > 1.0f)
            menu_alpha = 1.0f;
    }
    if (last_tab != active_tab)
    {
        fade_alpha = 0.0f;
        slide_offset = (active_tab > last_tab) ? 20.0f : -20.0f;
        last_tab = active_tab;
    }
    fade_alpha += ImGui::GetIO().DeltaTime * consts::ANIM_SPEED_FADE;
    if (fade_alpha > 1.0f)
        fade_alpha = 1.0f;
    slide_offset *= 0.85f;
    if (std::abs(slide_offset) < 0.1f)
        slide_offset = 0.0f;
    float eased_alpha = ease_out_cubic(menu_alpha);
    if (eased_alpha < 0.01f)
        return;
    ImGui::SetNextWindowSize(ImVec2(consts::UI_WINDOW_W, consts::UI_WINDOW_H), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, eased_alpha);
    ImGui::Begin("##VorInternalMain", &menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (ImGui::BeginTable("##MainLayout", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("##Sidebar", ImGuiTableColumnFlags_WidthFixed, consts::UI_SIDEBAR_WIDTH);
        ImGui::TableSetupColumn("##Content", ImGuiTableColumnFlags_WidthFixed, consts::UI_CONTENT_WIDTH);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::BeginChild("##SidebarChild", ImVec2(0, 0), true);
        if (ui::tab(tr::visuals(cfg.language), active_tab == 0))
            active_tab = 0;
        if (ui::tab(tr::advanced(cfg.language), active_tab == 1))
            active_tab = 1;
        if (ui::tab(tr::options(cfg.language), active_tab == 2))
            active_tab = 2;
        if (ui::tab(tr::configs(cfg.language), active_tab == 3))
            active_tab = 3;
        ImGui::EndChild();
        ImGui::TableNextColumn();
        ImGui::BeginChild("##ContentChild", ImVec2(0, 0), true);
        float content_alpha = ease_out_cubic(fade_alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, content_alpha);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + slide_offset);
        if (active_tab == 0)
        {
            ui::checkbox(tr::enable(cfg.language), &cfg.esp_enabled);
            if (cfg.esp_enabled)
            {
                ImGui::Indent();
                ui::checkbox(tr::teammates(cfg.language), &cfg.esp_teammates);
                ui::checkbox(tr::outline(cfg.language), &cfg.esp_outline);
                if (cfg.esp_outline)
                {
                    ImGui::Indent();
                    ui::slider_float("##OutlineThick", &cfg.outline_thickness, consts::THICKNESS_SLIDER_MIN, consts::THICKNESS_SLIDER_MAX);
                    ImGui::Unindent();
                }
                ui::checkbox(tr::boxes(cfg.language), &cfg.show_boxes);
                if (cfg.show_boxes)
                {
                    ImGui::Indent();
                    const char *bt[] = {tr::frame(cfg.language), tr::corners(cfg.language)};
                    ui::combo("##BoxType", &cfg.box_type, bt, 2);
                    ui::slider_float("##BoxThick", &cfg.box_thickness, consts::THICKNESS_SLIDER_MIN, consts::THICKNESS_SLIDER_MAX);
                    ui::slider_float("##BoxPadding", &cfg.box_padding, consts::SIZE_SLIDER_MIN, consts::SIZE_SLIDER_MAX);
                    ImGui::Unindent();
                }
                ui::checkbox(tr::hp_bar(cfg.language), &cfg.show_health);
                if (cfg.show_health)
                {
                    ImGui::Indent();
                    const char *hp[] = {tr::left(cfg.language), tr::right(cfg.language), tr::top(cfg.language), tr::bottom(cfg.language)};
                    ui::combo("##HpPos", &cfg.hp_bar_pos, hp, 4);
                    ui::slider_float("##HpThick", &cfg.hp_bar_thickness, consts::THICKNESS_SLIDER_MIN, consts::THICKNESS_SLIDER_MAX);
                    ui::slider_float("##HpOffset", &cfg.hp_bar_offset, consts::SIZE_SLIDER_MIN, consts::SIZE_SLIDER_MAX);
                    ImGui::Unindent();
                }
                ui::checkbox(tr::armor_bar(cfg.language), &cfg.show_armor_bar);
                if (cfg.show_armor_bar)
                {
                    ImGui::Indent();
                    const char *ap[] = {tr::left(cfg.language), tr::right(cfg.language), tr::top(cfg.language), tr::bottom(cfg.language)};
                    ui::combo("##ArmorPos", &cfg.armor_bar_pos, ap, 4);
                    ui::slider_float("##ArmorThick", &cfg.armor_bar_thickness, consts::THICKNESS_SLIDER_MIN, consts::THICKNESS_SLIDER_MAX);
                    ui::slider_float("##ArmorOffset", &cfg.armor_bar_offset, consts::SIZE_SLIDER_MIN, consts::SIZE_SLIDER_MAX);
                    ImGui::Unindent();
                }
                ui::checkbox(tr::names(cfg.language), &cfg.show_names);
                if (cfg.show_names)
                {
                    ImGui::Indent();
                    const char *np[] = {tr::top(cfg.language), tr::bottom(cfg.language), tr::left(cfg.language), tr::right(cfg.language)};
                    ui::combo("##NamePos", &cfg.name_pos, np, 4);
                    ImGui::Unindent();
                }
                ui::checkbox(tr::snaplines(cfg.language), &cfg.show_snaplines);
                if (cfg.show_snaplines)
                {
                    ImGui::Indent();
                    const char *st[] = {tr::bottom(cfg.language), tr::center(cfg.language), tr::top(cfg.language)};
                    ui::combo("##SnapOrigin", &cfg.snapline_type, st, 3);
                    ui::slider_float("##SnapThick", &cfg.snapline_thickness, consts::THICKNESS_SLIDER_MIN, consts::THICKNESS_SLIDER_MAX);
                    ImGui::Unindent();
                }
                ImGui::Separator();
                ui::slider_float("##TextSize", &cfg.text_size, consts::TEXT_SIZE_MIN, consts::TEXT_SIZE_MAX);
                ui::slider_float("##TextOutline", &cfg.text_outline_thickness, 0.0f, consts::THICKNESS_SLIDER_MAX);
                ui::slider_float("##TextPadding", &cfg.text_padding, consts::SIZE_SLIDER_MIN, consts::SIZE_SLIDER_MAX);
                ImGui::Separator();
                ui::color_edit(tr::col_box_t(cfg.language), cfg.color_box_t);
                ui::color_edit(tr::col_box_ct(cfg.language), cfg.color_box_ct);
                ui::color_edit(tr::col_snap_t(cfg.language), cfg.color_snap_t);
                ui::color_edit(tr::col_snap_ct(cfg.language), cfg.color_snap_ct);
                ui::color_edit(tr::col_text(cfg.language), cfg.color_text);
                ui::color_edit(tr::col_armor(cfg.language), cfg.color_armor);
                ImGui::Unindent();
            }
        }
        else if (active_tab == 1)
        {
            ui::checkbox(tr::watermark(cfg.language), &cfg.show_watermark);
            ui::checkbox(tr::armor(cfg.language), &cfg.show_armor);
            ui::checkbox(tr::distance(cfg.language), &cfg.show_distance);
            ui::checkbox(tr::defuse_kit(cfg.language), &cfg.show_defuse_kit);
            ui::checkbox(tr::scoped(cfg.language), &cfg.show_scoped);
            ui::checkbox(tr::flash(cfg.language), &cfg.show_flash);
            ui::checkbox(tr::offscreen(cfg.language), &cfg.show_offscreen_arrows);
        }
        else if (active_tab == 2)
        {
            const char *langs[] = {"English", "Русский"};
            ui::combo("##Lang", &cfg.language, langs, 2);

            ImGui::Separator();
            if (ui::button(tr::unload(cfg.language), ImGui::GetContentRegionAvail().x))
            {
                g_bUnloadRequested.store(true);
            }
        }
        else if (active_tab == 3)
        {
            ui::input_text("##CfgName", cfg_input, consts::MAX_CONFIG_NAME_LENGTH);
            cfg_name = cfg_input;
            float btn_w = (ImGui::GetContentRegionAvail().x - consts::UI_SPACE * 2.0f) / 3.0f;
            if (ui::button(tr::save(cfg.language), btn_w))
                config_manager::save(cfg_name, cfg);
            ImGui::SameLine();
            if (ui::button(tr::load(cfg.language), btn_w))
                config_manager::load(cfg_name, cfg);
            ImGui::SameLine();
            if (ui::button(tr::del(cfg.language), btn_w))
                config_manager::remove(cfg_name);
            ImGui::Separator();
            ui::text(tr::saved_configs(cfg.language));
            ImGui::BeginChild("##ConfigList", ImVec2(0, 100.0f), true);
            const auto configs = config_manager::list();
            for (const auto &n : configs)
            {
                if (ui::selectable(n.c_str(), cfg_name == n))
                {
                    cfg_name = n;
                    const std::size_t copy_len = std::min(cfg_name.length(), static_cast<std::size_t>(consts::MAX_CONFIG_NAME_LENGTH - 1));
                    std::memcpy(cfg_input, cfg_name.data(), copy_len);
                    cfg_input[copy_len] = '\0';
                    config_manager::load(cfg_name, cfg);
                }
            }
            ImGui::EndChild();
        }
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::EndTable();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void render::draw_watermark(ImDrawList *const dl)
{
    static float alpha = 0.0f;
    alpha += ImGui::GetIO().DeltaTime * consts::ANIM_SPEED_WATERMARK;
    if (alpha > 1.0f)
        alpha = 1.0f;
    float eased_alpha = ease_out_cubic(alpha);
    if (eased_alpha < 0.01f)
        return;
    const char *wm_text = "VorInternal";
    ImFont *font = ImGui::GetFont();
    ImVec2 text_size = font->CalcTextSizeA(consts::WATERMARK_FONT_SIZE, consts::MAX_TEXT_WIDTH, 0.0f, wm_text);
    float w = text_size.x + consts::WATERMARK_PAD_X * 2.0f;
    float h = text_size.y + consts::WATERMARK_PAD_Y * 2.0f;
    float rounding = h / 2.0f;
    ImVec2 screen_size = ImGui::GetIO().DisplaySize;
    ImVec2 pos(screen_size.x - w - consts::WATERMARK_MARGIN, consts::WATERMARK_MARGIN);
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), apply_alpha(consts::UI_COL_FRAME, eased_alpha), rounding);
    dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), apply_alpha(consts::UI_COL_BORDER, eased_alpha), rounding);
    dl->AddText(font, consts::WATERMARK_FONT_SIZE, ImVec2(pos.x + consts::WATERMARK_PAD_X, pos.y + consts::WATERMARK_PAD_Y), apply_alpha(consts::UI_COL_ACCENT, eased_alpha), wm_text);
}
