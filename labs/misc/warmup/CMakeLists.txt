cmake_minimum_required(VERSION 3.3)

project(lab)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
