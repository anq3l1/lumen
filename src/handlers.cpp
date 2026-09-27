#include "handlers.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>

namespace fs = std::filesystem;

std::vector<Application> applications;


void load_app()
{
    const char* home = std::getenv("HOME");

    if (home == nullptr)
    {
        std::cerr << "HOME environment variable not found.\n";
        return;
    }

    std::vector<fs::path> directories = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        fs::path(home) / ".local/share/applications"
    };

    for (const auto& directory : directories)
    {
        if (!fs::exists(directory) || !fs::is_directory(directory))
            continue;

        for (const auto& entry : fs::directory_iterator(directory))
        {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension() != ".desktop")
                continue;

            std::ifstream app(entry.path());

            if (!app.is_open())
                continue;

            Application application;
            std::string line;

            while (std::getline(app, line))
            {
                if (line == "[Desktop Entry]")
                    continue;

                if (!line.empty() && line[0] == '[')
                    break;

                if (line.rfind("Name=", 0) == 0)
                {
                    application.name = line.substr(5);
                }
                else if (line.rfind("Exec=", 0) == 0)
                {
                    application.exec = line.substr(5);
                }
                else if (line.rfind("Icon=", 0) == 0)
                {
                    application.icon = line.substr(5);
                }
            }

            if (application.name.empty())
                continue;
            if (application.icon.empty())
                continue;

            application.desktop_file = entry.path();

            applications.push_back(application);
        }
    }
}


void list_app()
{
    for (const auto& application : applications)
    {
        std::cout << application.name << '\n';
    }
}


std::string clean_exec(std::string exec)
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

    return exec;
}


std::vector<std::string> split_command(const std::string& command)
{
    std::vector<std::string> arguments;

    std::string current;
    bool inside_quotes = false;
    char quote = '\0';

    for (std::size_t i = 0; i < command.size(); ++i)
    {
        char c = command[i];

        if ((c == '"' || c == '\''))
        {
            if (!inside_quotes)
            {
                inside_quotes = true;
                quote = c;
            }
            else if (quote == c)
            {
                inside_quotes = false;
                quote = '\0';
            }

            continue;
        }

        if (std::isspace(static_cast<unsigned char>(c)) && !inside_quotes)
        {
            if (!current.empty())
            {
                arguments.push_back(current);
                current.clear();
            }

            continue;
        }
        if (c == '\\' && i + 1 < command.size())
        {
            ++i;
            current += command[i];
            continue;
        }

        current += c;
    }

    if (!current.empty())
        arguments.push_back(current);

    return arguments;
}


void open_app(std::string name)
{
    for (const auto& application : applications)
    {
        if (application.name == name)
        {
            std::cout << "Start: " << application.name << '\n';

            std::string exec = clean_exec(application.exec);

            std::vector<std::string> arguments = split_command(exec);

            if (arguments.empty())
            {
                std::cerr << "Invalid Exec field.\n";
                return;
            }

            std::vector<char*> argv;

            for (auto& argument : arguments)
                argv.push_back(argument.data());

            argv.push_back(nullptr);

            pid_t pid = fork();

            if (pid == -1)
            {
                std::cerr << "Failed to fork.\n";
                return;
            }

            if (pid == 0)
            {
                execvp(argv[0], argv.data());

                std::cerr << "Failed to start: "
                          << application.name << '\n';

                _exit(127);
            }
            return;
        }
    }

    std::cout << "Not find a " << name << "!\n";
}


std::string to_lower(std::string text)
{
    for (char& c : text)
    {
        c = static_cast<char>(
            std::tolower(static_cast<unsigned char>(c))
        );
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

        if (name_lower.find(low_que) != std::string::npos)
        {
            std::cout << "Found: "
                      << application.name
                      << '\n';

            ++found_count;
        }
    }

    if (found_count == 0)
        std::cout << "No applications found.\n";
}