#pragma once

#include <cstdlib>
#include <filesystem>

namespace lumen_paths
{
namespace fs = std::filesystem;

inline fs::path home()
{
    const char* value = std::getenv("HOME");
    return value && *value ? fs::path(value) : fs::current_path();
}

inline fs::path data_dir()
{
    return home() / ".local" / "share" / "lumen";
}

inline fs::path config_dir()
{
    return home() / ".config" / "lumen";
}

inline fs::path proc_file()
{
    return data_dir() / "proc.json";
}

inline fs::path stat_file()
{
    return data_dir() / "stat.json";
}

inline fs::path style_file()
{
    return config_dir() / "style.css";
}

inline void ensure_directories()
{
    std::error_code ec;
    fs::create_directories(data_dir(), ec);
    fs::create_directories(config_dir(), ec);
}
}
