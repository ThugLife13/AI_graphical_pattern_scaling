cmake_minimum_required(VERSION 3.20)
project(AI_graphical_pattern_scaling)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

add_executable(AI_graphical_pattern_scaling
        core/src/main.cpp
        GUI/mainFrame/src/setup.cpp
        GUI/mainFrame/lib/setup.h
        GUI/mainFrame/src/mainFrame.cpp
        GUI/mainFrame/lib/mainFrame.h
        GUI/mainFrame/src/mainFrameEventHandlers.cpp
        GUI/mainFrame/lib/mainFrameEventHandlers.h
        GUI/imageFrame/src/imageFrame.cpp
        GUI/imageFrame/lib/imageFrame.h
        GUI/imageFrame/src/imageFrameEventHandler.cpp
        GUI/imageFrame/lib/imageFrameEventHandler.h
        core/src/core.cpp
        core/lib/core.h)

find_package(wxWidgets CONFIG REQUIRED)

find_package(spdlog CONFIG REQUIRED)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(AI_graphical_pattern_scaling PRIVATE wx::core wx::base spdlog::spdlog ${OpenCV_LIBS})

target_compile_options(AI_graphical_pattern_scaling PRIVATE "/utf-8")

set_target_properties(AI_graphical_pattern_scaling PROPERTIES LINK_FLAGS "/ENTRY:WinMainCRTStartup")
