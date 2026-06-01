#include "config.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace config_manager
{
    std::filesystem::path get_dir()
    {
        char *appdata = nullptr;
        std::size_t len = 0;
        if (_dupenv_s(&appdata, &len, "APPDATA") == 0 && appdata)
        {
            std::filesystem::path p = std::filesystem::path(appdata) / "VorInternal";
            free(appdata);
            return p;
        }
        return std::filesystem::current_path() / "VorInternal";
    }

    void save(const std::string &name, const Config &cfg)
    {
        std::error_code ec;
        std::filesystem::path dir = get_dir();
        std::filesystem::create_directories(dir, ec);
        if (ec)
            return;
        std::ofstream f(dir / (name + ".cfg"), std::ios::binary);
        if (f.is_open())
        {
            f.write(reinterpret_cast<const char *>(&cfg), sizeof(Config));
        }
    }

    void load(const std::string &name, Config &cfg)
    {
        std::ifstream f(get_dir() / (name + ".cfg"), std::ios::binary);
        if (f.is_open())
        {
            Config tmp;
            f.read(reinterpret_cast<char *>(&tmp), sizeof(Config));
            if (tmp.magic == cfg.magic && tmp.version == cfg.version)
            {
                cfg = tmp;
            }
        }
    }

    void remove(const std::string &name)
    {
        std::error_code ec;
        std::filesystem::remove(get_dir() / (name + ".cfg"), ec);
    }

    std::vector<std::string> list()
    {
        std::vector<std::string> res;
        std::error_code ec;
        std::filesystem::path dir = get_dir();
        if (!std::filesystem::exists(dir, ec) || ec)
            return res;
        for (const auto &e : std::filesystem::directory_iterator(dir, ec))
        {
            if (ec)
                break;
            if (e.path().extension() == ".cfg")
            {
                res.push_back(e.path().stem().string());
            }
        }
        return res;
    }
}
