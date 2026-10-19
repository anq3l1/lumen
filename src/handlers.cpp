#include "handlers.hpp"
#include "stats.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <utility>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

std::vector<Application> applications;

namespace
{
std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string categories_for_display(std::string value)
{
    while (!value.empty() && value.back() == ';')
        value.pop_back();
    std::replace(value.begin(), value.end(), ';', ',');
    return value;
}

std::vector<std::string> split_command(const std::string& command)
{
    std::vector<std::string> arguments;
    std::string current;
    char quote = '\0';
    bool escaped = false;

    for (char c : command)
    {
        if (escaped)
        {
            current += c;
            escaped = false;
            continue;
        }
        if (c == '\\')
        {
            escaped = true;
            continue;
        }
        if (quote != '\0')
        {
            if (c == quote)
                quote = '\0';
            else
                current += c;
            continue;
        }
        if (c == '\'' || c == '"')
        {
            quote = c;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(c)))
        {
            if (!current.empty())
            {
                arguments.push_back(current);
                current.clear();
            }
            continue;
        }
        current += c;
    }
    if (escaped)
        current += '\\';
    if (!current.empty())
        arguments.push_back(current);

    const std::set<std::string> field_codes = {
        "%U", "%u", "%F", "%f", "%i", "%c", "%k", "%d", "%D", "%n", "%N", "%v", "%m"
    };
    for (auto& argument : arguments)
    {
        for (const auto& code : field_codes)
        {
            std::size_t position;
            while ((position = argument.find(code)) != std::string::npos)
                argument.erase(position, code.size());
        }
    }
    arguments.erase(std::remove_if(arguments.begin(), arguments.end(),
        [](const std::string& value) { return value.empty(); }), arguments.end());
    return arguments;
}
}

std::string to_lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

void load_app()
{
    applications.clear();

    const auto home_env = std::getenv("HOME");
    if (!home_env)
    {
        std::cerr << "Lumen: HOME environment variable not found.\n";
        return;
    }

    // User entries are loaded first; duplicate names from system directories
    // are ignored so local desktop files can override system ones.
    const std::vector<fs::path> directories = {
        fs::path(home_env) / ".local/share/applications",
        "/usr/local/share/applications",
        "/usr/share/applications"
    };
    std::set<std::string> seen_names;

    for (const auto& directory : directories)
    {
        std::error_code ec;
        if (!fs::is_directory(directory, ec))
            continue;

        fs::recursive_directory_iterator it(
            directory, fs::directory_options::skip_permission_denied, ec), end;
        for (; it != end; it.increment(ec))
        {
            if (ec)
            {
                ec.clear();
                continue;
            }
            if (!it->is_regular_file(ec) || it->path().extension() != ".desktop")
                continue;

            std::ifstream input(it->path());
            if (!input)
                continue;

            Application app;
            std::string line;
            std::string type = "Application";
            bool in_entry = false;
            bool hidden = false;
            bool no_display = false;

            while (std::getline(input, line))
            {
                line = trim(line);
                if (line.empty() || line[0] == '#')
                    continue;
                if (line.front() == '[')
                {
                    in_entry = line == "[Desktop Entry]";
                    continue;
                }
                if (!in_entry)
                    continue;

                const auto equal = line.find('=');
                if (equal == std::string::npos)
                    continue;
                const auto key = line.substr(0, equal);
                const auto value = line.substr(equal + 1);

                if (key == "Name") app.name = value;
                else if (key == "Exec") app.exec = value;
                else if (key == "Icon") app.icon = value;
                else if (key == "Categories") app.category = categories_for_display(value);
                else if (key == "Type") type = value;
                else if (key == "Hidden" && to_lower(value) == "true") hidden = true;
                else if (key == "NoDisplay" && to_lower(value) == "true") no_display = true;
            }

            if (app.name.empty() || app.exec.empty() || hidden || no_display || type != "Application")
                continue;

            const std::string key = to_lower(app.name);
            if (!seen_names.insert(key).second)
                continue;

            app.desktop_file = it->path();
            app.time = 0;
            applications.push_back(std::move(app));
        }
    }

    rebuildAppStatistics();
    for (auto& app : applications)
        app.time = readTimeFromStatistic(app.name);

    std::sort(applications.begin(), applications.end(),
        [](const Application& a, const Application& b) {
            return to_lower(a.name) < to_lower(b.name);
        });
}

void list_app()
{
    for (const auto& application : applications)
        std::cout << application.name << " time: " << application.time << " s.\n";
}

void open_app(std::string name)
{
    const auto found = std::find_if(applications.begin(), applications.end(),
        [&name](const Application& app) { return app.name == name; });

    if (found == applications.end())
    {
        std::cerr << "Lumen: application not found: " << name << '\n';
        return;
    }

    auto arguments = split_command(found->exec);
    if (arguments.empty())
    {
        std::cerr << "Lumen: invalid Exec field for " << found->name << '\n';
        return;
    }

    std::vector<char*> argv;
    argv.reserve(arguments.size() + 1);
    for (auto& argument : arguments)
        argv.push_back(argument.data());
    argv.push_back(nullptr);

    const pid_t pid = fork();
    if (pid < 0)
    {
        perror("Lumen: fork");
        return;
    }
    if (pid == 0)
    {
        // Avoid leaving a child attached to the GUI's process group.
        setsid();
        execvp(argv[0], argv.data());
        perror("Lumen: execvp");
        _exit(127);
    }
}

void search_app(std::string query)
{
    const auto needle = to_lower(query);
    std::size_t count = 0;

    for (const auto& app : applications)
    {
        if (to_lower(app.name).find(needle) != std::string::npos)
        {
            std::cout << "Found: " << app.name << '\n';
            ++count;
        }
    }

    if (count == 0)
        std::cout << "No applications found.\n";
}
