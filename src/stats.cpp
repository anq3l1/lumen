#include <iostream>

#include <fstream>
#include <nlohmann/json.hpp>
#include <map>

//#include "handlers.hpp"

using json = nlohmann::json;

std::map<std::string, std::string> applications_execs;

std::string clean_exe(std::string exec)
{

    const std::vector<std::string> placeholders = {
        "%U",
        "%u",
        "%F",
        "%f",
        "%i",
        "%c",
        "%k"
    };

    for (const auto& placeholder : placeholders)
    {
        std::size_t pos;

        while ((pos = exec.find(placeholder)) != std::string::npos)
        {
            exec.erase(pos, placeholder.length());
        }
    }

    std::istringstream iss(exec);

    std::string exe;
    iss >> exe;

    exe = std::filesystem::path(exe).filename().string();

    return exe;
}

void addInJson(std::string appexec, std::string appname)
{
    std::ifstream procfile("proc.json");

    if (!procfile.is_open())
    {
        std::cerr << "Cannot open proc.json\n";
        return;
    }

    json j = json::parse(procfile);

    std::fstream statfile("stat.json");

    json stat = json::object();

    if (statfile.is_open() &&
        statfile.peek() != std::ifstream::traits_type::eof())
    {
        statfile >> stat;
    }

    statfile.close();

    std::string clean_ex = clean_exe(appexec);

    for (auto& [name, time] : j.items())
    {
        std::string lower_name = name;
        std::string lower_ex = clean_ex;

        for (char& c : lower_name)
            c = std::tolower(c);

        for (char& c : lower_ex)
            c = std::tolower(c);


        if (lower_ex.find(lower_name) != std::string::npos)
        {
            applications_execs[appname] = lower_name;

            stat[appname] = time;

            std::ofstream outfile("stat.json");
            outfile << stat.dump(4);

            break;
        }
    }
}

int readTimeFromStatistic(const std::string& applicationname)
{
    std::ifstream statfile("stat.json");

    if (!statfile.is_open())
        return 0;

    json j = json::parse(statfile);

    for (auto& [name, time] : j.items())
    {
        if (name == applicationname)
        {
            return time.get<int>();
        }
    }

    return 0;
}