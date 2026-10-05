#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <format>
#include <map>
#include <thread>
#include <chrono>

#include <nlohmann/json.hpp>


//---------NAMESPACES---------

using namespace std::literals;
namespace fs = std::filesystem;
using json = nlohmann::json;

void addInStatictic(std::string name, int time)
{
    if (name.empty())
        return;

    json data;

    std::ifstream read_stat("proc.json");

    if (read_stat.is_open() && read_stat.peek() != std::ifstream::traits_type::eof())
    {
        read_stat >> data;
    }

    read_stat.close();

    if (!data.contains(name))
    {
        data[name] = time;
    }
    else
    {
        data[name] = data[name].get<int>() + time;
    }

    std::ofstream statistic("proc.json");

    statistic << data.dump(4);
}

std::string getProcessName(int pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/comm";

    std::fstream file(path);

    if(!file.is_open())
        return "";

    std::string process_name;

    std::getline(file, process_name);

    return process_name;
}

std::map<int, std::string> getProcesses()
{
    std::map<int, std::string> processes;

    for (const auto& entry : fs::directory_iterator("/proc"))
    {
        if (!entry.is_directory())
            continue;

        std::string pid = entry.path().filename().string();

        if (pid.empty() ||
            !std::all_of(pid.begin(), pid.end(), ::isdigit))
            continue;

        processes[(std::stoi(pid))] = getProcessName(std::stoi(pid));
    }

    return processes;
}

int ProcessManager()
{
    std::ifstream check("proc.json");

    if (!check.good())
    {
        std::ofstream file("proc.json");
        file << "{}";
    }

    std::map<int, std::chrono::steady_clock::time_point> startTime;

    std::map<int, std::string> oldprocess = getProcesses();

    for(auto& [pid, name] : oldprocess)
    {
        startTime[pid] = std::chrono::steady_clock::now();
    }

    while(true)
    {
        oldprocess = getProcesses();

        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::map<int, std::string> newprocess = getProcesses();

        for(auto& [pid, name] : newprocess)
        {
            if(!oldprocess.contains(pid))
            {
                startTime[pid] = std::chrono::steady_clock::now();
            }
        }

        for(auto& [pid, name] : oldprocess)
        {
            auto start = startTime[pid];

            if(!newprocess.contains(pid))
            {
                auto end = std::chrono::steady_clock::now();

                auto time = (end - start) / 1s;

                addInStatictic(name, time);
            }

        }
    }
    

    return 0;
}