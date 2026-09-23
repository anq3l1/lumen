#include <iostream>
#include <fstream>
#include <filesystem>

#include "handlers.hpp"

int main(int argc, char* argv[])
{
    load_app();

    if(argc > 1 && std::string(argv[1]) == "--list")
        list_app();
    else if(argc > 2 && std::string(argv[1]) == "--open")
    {
        std::string name = argv[2];

        open_app(name);
    }
    else if(argc > 2 && std::string(argv[1]) == "--search")
    {
        std::string query = argv[2];

        search_app(query);
    }
}