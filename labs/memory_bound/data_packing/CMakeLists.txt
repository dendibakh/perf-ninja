cmake_minimum_required(VERSION 3.3)

project(lab)

# mitigate the impact of code alignment
set(CMAKE_CXX_FLAGS "-mllvm -align-all-functions=6 ${CMAKE_CXX_FLAGS}")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
