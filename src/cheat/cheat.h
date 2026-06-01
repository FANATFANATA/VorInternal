#pragma once
#include "../types.h"
#include <atomic>

namespace cheat
{
    void worker(SharedState &s, const Config &cfg, std::atomic<bool> &is_running);
}
