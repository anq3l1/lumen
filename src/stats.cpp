#include <iostream>

#include <fstream>
#include <nlohmann/json.hpp>
#include <map>

using json = nlohmann::json;

std::map<std::string, std::string> applications;

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

    json j = json::parse(procfile);

    std::ifstream statfile("stat.json");

    json stat = json::object();

    if (statfile.is_open() && statfile.peek() != std::ifstream::traits_type::eof())
    {
        statfile >> stat;
    }

    statfile.close();

    for (auto& [name, time] : j.items())
    {
        std::string lower_name = name;

        std::string clean_ex = clean_exe(appexec);

        for (char& c : lower_name)
            c = std::tolower(c);

        for (char& c : clean_ex)
            c = std::tolower(c);

        if (clean_ex.find(lower_name) != std::string::npos)
        {
            applications[appname] = lower_name; // Добавляет имя приложения и имя процесса в словарь.

            stat[appname] = time;
            
            std::ofstream statfile("stat.json");
            statfile << stat.dump(4);
        }
    }
}

void readFromStatistic()
{

}