cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(MyTuple LANGUAGES CXX)

add_executable(out main.cpp)
target_include_directories(out PRIVATE include)

target_compile_options(out PRIVATE -Wall -Wpedantic -Wextra )
