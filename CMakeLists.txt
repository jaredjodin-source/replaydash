cmake_minimum_required(VERSION 3.25)

project(ReplayDash VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include($ENV{GEODE_SDK}/cmake/Geode.cmake)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/Macro.cpp
    src/RDM.cpp
    src/GDR.cpp
    src/GDR2.cpp
)

setup_geode_mod(${PROJECT_NAME})
