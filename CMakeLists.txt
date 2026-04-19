# 
# Emilio Cazares Borbon 04/19/2026
# 
# Building the Clustering project with CMake.
# 
# Created for Georgetown University Data Mining COSC 3590, Spring 2026.
# 
cmake_minimum_required(VERSION 3.22)
project(Clustering VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# libraries
add_subdirectory(external/arff)

add_library(Clustering_processor_lib)
target_sources(Clustering_processor_lib
    PRIVATE
        src/clustering_processor.cxx
)
target_include_directories(Clustering_processor_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(Clustering_processor_lib
    PUBLIC
        arff_lib
)

# test executable
add_executable(processor_test)
target_sources(processor_test
    PRIVATE
        src/tests.cxx
)

target_link_libraries(processor_test
    PRIVATE
        Clustering_processor_lib
        arff_lib
)

target_compile_options(processor_test
    PRIVATE
        -Wall
        -Wextra
        -Wfloat-conversion
)

# main executable
add_executable(processor_main)
target_sources(processor_main
    PRIVATE
        src/main.cxx
)

target_link_libraries(processor_main
    PRIVATE
        Clustering_processor_lib
        arff_lib
)

target_compile_options(processor_main
    PRIVATE
        -Wall
        -Wextra
        -Wfloat-conversion
)
