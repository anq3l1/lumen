#include "processmanager.hpp"
#include "stats.hpp"
#include "paths.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace
{
std::string get_process_name(const fs::path& proc_directory)
{
    std::ifstream input(proc_directory / "comm");
    std::string name;
    if (input)
        std::getline(input, name);
    return name;
}

std::map<pid_t, std::string> get_processes()
{
    std::map<pid_t, std::string> processes;
    std::error_code ec;
    fs::directory_iterator it("/proc", fs::directory_options::skip_permission_denied, ec), end;

    for (; it != end; it.increment(ec))
    {
        if (ec)
        {
            ec.clear();
            continue;
        }
        if (!it->is_directory(ec))
            continue;

        const std::string filename = it->path().filename().string();
        if (filename.empty() || !std::all_of(filename.begin(), filename.end(),
            [](unsigned char c) { return std::isdigit(c); }))
            continue;

        try
        {
            const auto pid = static_cast<pid_t>(std::stol(filename));
            const auto name = get_process_name(it->path());
            if (!name.empty())
                processes.emplace(pid, name);
        }
        catch (const std::exception&)
        {
            continue;
        }
    }
    return processes;
}
}

int ProcessManager()
{
    lumen_paths::ensure_directories();
    std::cout << "Lumen process manager started. Data: "
              << lumen_paths::data_dir() << std::endl;

    int seconds_since_rebuild = 0;
    const pid_t own_pid = getpid();

    while (true)
    {
        std::this_thread::sleep_for(1s);
        const auto processes = get_processes();
        std::set<std::string> unique_names;

        for (const auto& [pid, name] : processes)
        {
            if (pid == own_pid || name == "lumen")
                continue;
            unique_names.insert(name);
        }

        const std::vector<std::string> names(unique_names.begin(), unique_names.end());
        addProcessTimes(names, 1);

        if (++seconds_since_rebuild >= 10)
        {
            rebuildAppStatistics();
            seconds_since_rebuild = 0;
        }
    }

    return 0;
}
