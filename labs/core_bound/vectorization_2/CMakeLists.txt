cmake_minimum_required(VERSION 3.3)

project(lab)

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
# target_compile_options(lab PRIVATE "-funroll-loops" "-Rpass=." "-Rpass-analysis=.")
target_compile_options(lab PRIVATE "-Rpass=loop-vectorize" "-Rpass-missed=loop-vectorize" "-Rpass-analysis=loop-vectorize")
