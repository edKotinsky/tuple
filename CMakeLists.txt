cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TUPLE_TEST "Compile and run tests" OFF)

project(Tuple LANGUAGES CXX)

add_library(Tuple INTERFACE)
target_include_directories(Tuple INTERFACE include)

target_compile_options(Tuple INTERFACE -Wall -Wpedantic -Wextra )

if(GZTUPLE_TEST)
    add_subdirectory(test)
endif()
