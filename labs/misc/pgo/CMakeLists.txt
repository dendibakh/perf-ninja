cmake_minimum_required(VERSION 3.3)

project(lab)

include_directories(lua)
file(GLOB EXT_LAB_srcs lua/*.c)
file(GLOB EXT_VALIDATE_srcs lua/*.c)

#set(CMAKE_C_FLAGS "-fprofile-instr-generate ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "-fprofile-instr-use=\"${CMAKE_CURRENT_SOURCE_DIR}/code.profdata\" ${CMAKE_C_FLAGS}")

set(VALIDATE_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/reference_output.txt")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
