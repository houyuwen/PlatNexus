cmake_minimum_required(VERSION 3.10)
project(utils C)

add_library(utils STATIC)
target_include_directories(utils PUBLIC include)
