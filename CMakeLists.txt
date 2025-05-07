cmake_minimum_required(VERSION 3.15)
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
        core/lib/core.h
        core/src/imageProcessing.cpp
        core/lib/imageProcessing.h
        GUI/decompositionFrame/src/decompositionFrame.cpp
        GUI/decompositionFrame/lib/decompositionFrame.h
        GUI/decompositionFrame/src/decompositionFrameEventHandler.cpp
        GUI/decompositionFrame/lib/decompositionFrameEventHandler.h
        core/src/imagePanel.cpp
        core/lib/imagePanel.h)

find_package(wxWidgets CONFIG REQUIRED)

find_package(spdlog CONFIG REQUIRED)

find_package(OpenCV REQUIRED)

find_package(nlohmann_json REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(AI_graphical_pattern_scaling PRIVATE wx::core wx::base spdlog::spdlog ${OpenCV_LIBS} nlohmann_json::nlohmann_json)

target_compile_options(AI_graphical_pattern_scaling PRIVATE "/utf-8")

set_target_properties(AI_graphical_pattern_scaling PROPERTIES LINK_FLAGS "/ENTRY:WinMainCRTStartup")
