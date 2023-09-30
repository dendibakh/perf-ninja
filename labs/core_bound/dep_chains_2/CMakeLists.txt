cmake_minimum_required(VERSION 2.8.12)

project(lab)

# Disable SLP vectorization to isolate the effect of breaking a dependency chain.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-slp-vectorize")
# Clear the -O3 flag in the CMAKE_CXX_FLAGS_RELEASE, because it will reenable SLP vectorization.
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
