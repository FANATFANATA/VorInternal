#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../types.h"
#include "imgui.h"
#include <string>

namespace render
{
    void setup_monochrome();
    void draw_esp(ImDrawList *const dl, const std::vector<PlayerData> &players, const Config &cfg, const ViewMatrix &vm, int sw, int sh);
    void draw_menu(Config &cfg, bool &menu, char *cfg_input, std::string &cfg_name);
    void draw_watermark(ImDrawList *const dl);
}
