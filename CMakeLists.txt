cmake_minimum_required(VERSION 3.20)
project(AI_graphical_pattern_scaling)

set(CMAKE_CXX_STANDARD 26)


if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup")
else()
    add_link_options(-mwindows)
endif()

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
        core/lib/imagePanel.h
)


find_package(wxWidgets CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)
#find_package(OpenMP REQUIRED)

target_include_directories(AI_graphical_pattern_scaling PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(AI_graphical_pattern_scaling PRIVATE
        wx::core
        wx::base
        spdlog::spdlog
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)