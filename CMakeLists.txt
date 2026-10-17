cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(BuildHelperAI VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/AIManager.cpp
    src/WebSearch.cpp
    src/TriggerHelper.cpp
    src/BuildHelperLayer.cpp
    src/DesignPickerLayer.cpp
    src/SettingsLayer.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK not set")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
include(GeodeSDK)
target_link_libraries(${PROJECT_NAME} geode-sdk)
CPMAddPackage("gh:nlohmann/json@3.11.3")
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
setup_geode_mod(${PROJECT_NAME})
