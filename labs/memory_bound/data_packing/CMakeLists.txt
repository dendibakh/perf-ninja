cmake_minimum_required(VERSION 2.8.12)

project(lab)

# mitigate the impact of code alignment
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "-mllvm -align-all-functions=6 ${CMAKE_CXX_FLAGS}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "-falign-functions=6 ${CMAKE_CXX_FLAGS}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-mllvm -align-all-functions=6 ${CMAKE_CXX_FLAGS}")
endif()

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
