#include "stats.hpp"
#include "paths.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace
{
std::string lower_copy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

json read_json_object(const fs::path& path)
{
    std::ifstream input(path);
    if (!input)
        return json::object();

    try
    {
        json value;
        input >> value;
        return value.is_object() ? value : json::object();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Lumen: cannot parse " << path << ": "
                  << error.what() << '\n';
        return json::object();
    }
}

void write_json_atomic(const fs::path& path, const json& value)
{
    lumen_paths::ensure_directories();
    const fs::path temporary = path.string() + ".tmp";

    {
        std::ofstream output(temporary, std::ios::trunc);
        if (!output)
        {
            std::cerr << "Lumen: cannot write " << temporary << '\n';
            return;
        }
        output << value.dump(4) << '\n';
        if (!output)
        {
            std::cerr << "Lumen: failed while writing " << temporary << '\n';
            return;
        }
    }

    std::error_code ec;
    fs::rename(temporary, path, ec);
    if (ec)
    {
    
        fs::remove(path, ec);
        ec.clear();
        fs::rename(temporary, path, ec);
    }

    if (ec)
    {
        std::cerr << "Lumen: cannot replace " << path << ": "
                  << ec.message() << '\n';
        fs::remove(temporary, ec);
    }
}


const std::unordered_map<std::string, std::string>& process_aliases()
{
    static const std::unordered_map<std::string, std::string> aliases = {
        {"kitten", "kitty"},
        {"kitty", "kitty"},
        {"chrome", "google-chrome"},
        {"google-chrome", "google-chrome"},
        {"google-chrome-stable", "google-chrome"},
        {"chromium", "chromium"},
        {"chromium-browser", "chromium"},
        {"code", "code"},
        {"code-oss", "code"},
        {"telegram", "telegram-desktop"},
        {"telegram-desktop", "telegram-desktop"},
        {"firefox", "firefox"},
        {"firefox-bin", "firefox"},
        {"brave", "brave-browser"},
        {"brave-browser", "brave-browser"},
        {"vivaldi-bin", "vivaldi"},
        {"vivaldi", "vivaldi"}
    };
    return aliases;
}

std::string canonical_executable(std::string name)
{
    name = lower_copy(trim(name));
    if (name.empty())
        return {};

    name = fs::path(name).filename().string();
    const auto& aliases = process_aliases();
    const auto found = aliases.find(name);
    return found == aliases.end() ? name : found->second;
}

std::vector<std::string> tokenize_exec(const std::string& command)
{
    std::vector<std::string> tokens;
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
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }
        current += c;
    }
    if (escaped)
        current += '\\';
    if (!current.empty())
        tokens.push_back(current);

    for (auto& token : tokens)
    {
        for (std::size_t i = 0; i < token.size();)
        {
            if (token[i] == '%' && i + 1 < token.size())
                token.erase(i, 2);
            else
                ++i;
        }
    }
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
        [](const std::string& token) { return token.empty(); }), tokens.end());

    if (!tokens.empty() && fs::path(tokens.front()).filename() == "env")
    {
        std::size_t index = 1;
        while (index < tokens.size() &&
               (tokens[index].find('=') != std::string::npos ||
                tokens[index].rfind("--", 0) == 0))
            ++index;
        if (index < tokens.size())
            tokens.erase(tokens.begin(), tokens.begin() + static_cast<long>(index));
    }

    return tokens;
}

std::string executable_from_exec(const std::string& exec)
{
    const auto tokens = tokenize_exec(exec);
    if (tokens.empty())
        return {};
    return canonical_executable(tokens.front());
}

struct DesktopApplication
{
    std::string name;
    std::string exec;
    bool hidden = false;
};

std::vector<DesktopApplication> read_desktop_apps()
{
    const auto home = lumen_paths::home();
    const std::vector<fs::path> directories = {
        home / ".local/share/applications",
        "/usr/local/share/applications",
        "/usr/share/applications"
    };

    std::vector<DesktopApplication> result;
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

            DesktopApplication app;
            std::string line;
            bool in_entry = false;
            bool type_application = true;
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
                else if (key == "Hidden" && lower_copy(value) == "true") app.hidden = true;
                else if (key == "NoDisplay" && lower_copy(value) == "true") no_display = true;
                else if (key == "Type" && value != "Application") type_application = false;
            }

            if (app.name.empty() || app.exec.empty() || app.hidden || no_display || !type_application)
                continue;

            const auto name_key = lower_copy(app.name);
            if (!seen_names.insert(name_key).second)
                continue;

            result.push_back(std::move(app));
        }
    }
    return result;
}
}

void addProcessTimes(const std::vector<std::string>& process_names, int seconds)
{
    if (seconds <= 0 || process_names.empty())
        return;

    lumen_paths::ensure_directories();
    json processes = read_json_object(lumen_paths::proc_file());

    for (const auto& name : process_names)
    {
        if (name.empty())
            continue;
        int current = 0;
        if (processes.contains(name) && processes[name].is_number_integer())
            current = processes[name].get<int>();
        else if (processes.contains(name) && processes[name].is_number())
            current = static_cast<int>(processes[name].get<double>());
        processes[name] = std::max(0, current) + seconds;
    }

    write_json_atomic(lumen_paths::proc_file(), processes);
}

void addLaunchesCount(const std::string& appnames, int launch)
{
    lumen_paths::ensure_directories();
    json launches = read_json_object(lumen_paths::launch_file());

    if (appnames.empty())
        return;

    int current = 0;

    if (launches.contains(appnames) && launches[appnames].is_number_integer())
        current = launches[appnames].get<int>();

    else if (launches.contains(appnames) && launches[appnames].is_number())
        current = static_cast<int>(launches[appnames].get<double>());

    launches[appnames] = std::max(0, current) + launch;

    write_json_atomic(lumen_paths::launch_file(), launches);
}

void rebuildAppStatistics()
{
    lumen_paths::ensure_directories();
    const json processes = read_json_object(lumen_paths::proc_file());

    json previous = read_json_object(lumen_paths::stat_file());
    json statistics = json::object();

    std::map<std::string, long long> canonical_times;

    for (auto it = processes.begin(); it != processes.end(); ++it)
    {
        if (!it.value().is_number())
            continue;
        const auto canonical = canonical_executable(it.key());
        if (!canonical.empty())
            canonical_times[canonical] = std::max(
                canonical_times[canonical],
                std::max<long long>(0, it.value().get<long long>())
            );
    }

    for (const auto& app : read_desktop_apps())
    {
        const auto executable = executable_from_exec(app.exec);
        const auto found = canonical_times.find(executable);
        if (found != canonical_times.end())
            statistics[app.name] = found->second;
        else if (previous.contains(app.name) && previous[app.name].is_number())
            statistics[app.name] = previous[app.name];
        else
            statistics[app.name] = 0;
    }

    write_json_atomic(lumen_paths::stat_file(), statistics);
}

int readTimeFromStatistic(const std::string& application_name)
{
    lumen_paths::ensure_directories();

    const json statistics = read_json_object(lumen_paths::stat_file());
    const auto found = statistics.find(application_name);

    if (found == statistics.end() || !found->is_number())
        return 0;

    return static_cast<int>(std::max<long long>(0, found->get<long long>()));
}

int readLaunchFromStatistic(const std::string& appname)
{
    lumen_paths::ensure_directories();

    const json launches = read_json_object(lumen_paths::launch_file());
    const auto found = launches.find(appname);

    if(found == launches.end() || !found->is_number())
        return 0;

    return static_cast<int>(std::max<long long>(0, found->get<long long>()));
}
