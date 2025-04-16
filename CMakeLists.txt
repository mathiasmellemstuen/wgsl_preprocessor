cmake_minimum_required(VERSION 3.10)

project(wgsl_preprocessor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(wgsl_preprocessor
    src/main.cpp
)