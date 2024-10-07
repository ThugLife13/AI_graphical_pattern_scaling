cmake_minimum_required(VERSION 3.29)
project(AI_graphical_pattern_scaling)

set(CMAKE_CXX_STANDARD 26)

add_executable(AI_graphical_pattern_scaling
        src/main.cpp
        src/core.cpp
        lib/core.h)

find_package(wxWidgets CONFIG REQUIRED)
target_link_libraries(AI_graphical_pattern_scaling PRIVATE wx::core wx::base)