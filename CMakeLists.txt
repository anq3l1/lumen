cmake_minimum_required(VERSION 3.16)

project(Lumen VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(lumen
    src/main.cpp
    src/handlers.cpp
    src/gui.cpp
)

target_include_directories(lumen PRIVATE
    include
)

target_link_libraries(lumen PRIVATE
    Qt6::Widgets
)