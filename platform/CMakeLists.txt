cmake_minimum_required(VERSION 3.10)
project(platform C)

add_library(platform STATIC)

target_include_directories(platform PUBLIC
    osal/include
    hal/include
    fs/include
    net/include
)

# Note: platform sources are selected by top-level PLAT_OS / PLAT_HW in root CMake
