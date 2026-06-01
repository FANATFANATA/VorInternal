#pragma once
#include "../types.h"

namespace math
{
    inline bool w2s(const Vector3 &world, Vector3 &screen, const ViewMatrix &vm, int w, int h)
    {
        constexpr float near_plane = 0.01f;
        constexpr float half = 2.0f;
        float clip_w = vm.matrix[3][0] * world.x + vm.matrix[3][1] * world.y + vm.matrix[3][2] * world.z + vm.matrix[3][3];
        if (clip_w < near_plane)
            return false;
        float x = vm.matrix[0][0] * world.x + vm.matrix[0][1] * world.y + vm.matrix[0][2] * world.z + vm.matrix[0][3];
        float y = vm.matrix[1][0] * world.x + vm.matrix[1][1] * world.y + vm.matrix[1][2] * world.z + vm.matrix[1][3];
        const float half_w = static_cast<float>(w) / half;
        const float half_h = static_cast<float>(h) / half;
        screen.x = half_w + (x / clip_w * half_w);
        screen.y = half_h - (y / clip_w * half_h);
        return true;
    }
}
