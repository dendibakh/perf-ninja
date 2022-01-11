cmake_minimum_required(VERSION 2.8.12)

project(lab)

set(VALIDATE_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/golden.ppm")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
