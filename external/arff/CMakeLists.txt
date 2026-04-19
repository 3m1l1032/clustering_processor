# 
# Emilio Cazares Borbon 01/30/2026
# 
# Building the ARFF project with CMake.
# 
# Created for Georgetown University Data Mining COSC 3590, Spring 2026.
# 
# Reference: https://cmake.org/cmake/help/latest/guide/tutorial/index.html
# 
cmake_minimum_required(VERSION 3.22.1)
project(arff_lib VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(arff_lib
    src/arff.cxx)

target_include_directories(arff_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(arff_lib PUBLIC cxx_std_17)
target_compile_options(arff_lib 
    PRIVATE
        -Wall
        -Wfloat-conversion
)

option(ARFF_DEBUG "Enable debug mode for ARFF library" OFF)
if(ARFF_DEBUG)
    target_compile_definitions(arff_lib PUBLIC ARFF_MAIN_DEBUGGER)
endif()

#add_compile_definitions(ARFF_MAIN_DEBUGGER)