cmake_minimum_required(VERSION 3.16)
project(lumen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(lumen
    src/main.cpp
    src/gui.cpp
    src/handlers.cpp
    src/stats.cpp
    processmanager/processmanager.cpp
)

target_include_directories(lumen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lumen PRIVATE PkgConfig::GTKMM nlohmann_json::nlohmann_json)
target_compile_options(lumen PRIVATE -Wall -Wextra -Wpedantic)
install(TARGETS lumen RUNTIME DESTINATION bin)
