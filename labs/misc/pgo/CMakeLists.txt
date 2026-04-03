cmake_minimum_required(VERSION 3.13)

project(lab)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-use=\"${CMAKE_CURRENT_SOURCE_DIR}/lua.profdata\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use=\"${CMAKE_CURRENT_SOURCE_DIR}/lua.profdata\"")

include_directories(lua)
file(GLOB EXT_LAB_srcs lua/*.c)
file(GLOB EXT_VALIDATE_srcs lua/*.c)

set(VALIDATE_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/reference_output.txt")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
