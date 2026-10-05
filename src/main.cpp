#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>

#include "handlers.hpp"
#include "processmanager.hpp"

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string(argv[1]) == "--processmanager")
    {
        ProcessManager();
    }
    else
    {
        load_app();

        if (argc > 1 && std::string(argv[1]) == "--list")
            list_app();
        else if (argc > 2 && std::string(argv[1]) == "--open")
            open_app(argv[2]);
        else if (argc > 2 && std::string(argv[1]) == "--search")
            search_app(argv[2]);
    }
}