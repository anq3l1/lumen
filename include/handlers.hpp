#pragma once

#include <string>
#include <vector>
#include <filesystem>

struct Application
{
    std::string name;
    std::string exec;
    std::string icon;
    int time;

    std::filesystem::path desktop_file;
};

extern std::vector<Application> applications;

void load_app();
void list_app();
void open_app(std::string name);
void search_app(std::string query);
std::string to_lower(std::string text);