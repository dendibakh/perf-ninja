cmake_minimum_required(VERSION 3.3)

project(lab)

set(CMAKE_CXX_STANDARD 17)

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)

add_executable(unit_tests unit_tests.cpp)
target_compile_definitions(unit_tests PRIVATE WITH_UNIT_TESTS)
