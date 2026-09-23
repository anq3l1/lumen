#include "handlers.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

struct Application
{
    std::string name;
    std::string exec;
    std::string icon;
};

std::vector<Application> applications;

void load_app()
{
    Application application;

    for (const auto& entry : fs::directory_iterator("/usr/share/applications"))
    {
        if (entry.path().extension() != ".desktop")
            continue;

        std::ifstream app(entry.path());

        if (!app.is_open())
            continue;

        std::string line;

        while (std::getline(app, line))
        {
            if (line.rfind("Name=", 0) == 0)
            {
                application.name = line.substr(5);
            }
            if (line.rfind("Exec=", 0) == 0)
            {
                application.exec = line.substr(5);
            }
            if (line.rfind("Icon=", 0) == 0)
            {
                application.icon = line.substr(5);
            }
        }
        applications.push_back(application);
    }
}

void list_app()
{
    Application application;

    for (const auto& application : applications)
    {
        std::cout << application.name << '\n';
    }
}

void open_app(std::string name)
{
    for (const auto& application : applications)
    {
        if (application.name == name)
        {
            std::cout << "Start: " << application.name << '\n';

            std::system(application.exec.c_str());
            return;
        }
    }
    std::cout << "Not find a " << name << "!\n";
}

std::string to_lower(std::string text)
{
    for(char& c : text)
    {
        c = std::tolower(c);
    }

    return text;
}

void search_app(std::string query)
{
    std::size_t found_count = 0;

    std::string low_que = to_lower(query);

    for (const auto& application : applications)
    {
        std::string name_lower = to_lower(application.name);
        std::size_t found = name_lower.find(low_que);

        if(found != std::string::npos)
        {
            std::cout << "Found: " << application.name << '\n';
            ++found_count;
        }
    }

    if(found_count == 0)
        std::cout << "No applications found.\n";
}