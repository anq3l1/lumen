#include "gui.hpp"
#include "handlers.hpp"
#include "processmanager.hpp"
#include "paths.hpp"
#include "stats.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    lumen_paths::ensure_directories();

    if (argc > 1 && std::string(argv[1]) == "--processmanager")
        return ProcessManager();

    if (argc == 1)
        return run_gui(argc, argv);

    load_app();

    const std::string command = argv[1];
    if (command == "--list")
        list_app();
    else if (command == "--open" && argc > 2)
        open_app(argv[2]);
    else if (command == "--search" && argc > 2)
        search_app(argv[2]);
    else if (command == "--help" || command == "-h")
    {
        std::cout << "Usage: lumen [--list | --open NAME | --search QUERY | --processmanager]\n";
    }
    else
    {
        std::cerr << "Usage: lumen [--list | --open NAME | --search QUERY | --processmanager]\n";
        return 1;
    }

    return 0;
}
