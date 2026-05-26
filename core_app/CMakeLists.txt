cmake_minimum_required(VERSION 3.10)
project(core_app C)

# Minimal placeholder CMake for core_app
add_library(core_app STATIC)
target_include_directories(core_app PUBLIC api)
target_sources(core_app PRIVATE
    src/main.c
)
