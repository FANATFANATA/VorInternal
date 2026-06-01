#pragma once
#include "../types.h"
#include <string>
#include <vector>

namespace config_manager
{
    void save(const std::string &name, const Config &cfg);
    void load(const std::string &name, Config &cfg);
    void remove(const std::string &name);
    std::vector<std::string> list();
}
