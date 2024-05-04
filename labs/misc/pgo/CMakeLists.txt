cmake_minimum_required(VERSION 3.3)

project(lab)

set(PROFILE_PHASE "" CACHE STRING "Either PRE or POST.")

include_directories(lua)
file(GLOB EXT_LAB_srcs lua/*.c)
file(GLOB EXT_VALIDATE_srcs lua/*.c)

set(VALIDATE_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/reference_output.txt")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)

if ("${PROFILE_PHASE}" STREQUAL "PRE")
    target_compile_options(lab PRIVATE "-fprofile-instr-generate")
    target_link_options(lab PRIVATE "-fprofile-instr-generate")
elseif ("${PROFILE_PHASE}" STREQUAL "POST")
    target_compile_options(lab PRIVATE "-fprofile-instr-use")
    target_link_options(lab PRIVATE "-fprofile-instr-use")
else()
    message(FATAL_ERROR "Invalid PROFILE_PHASE. Must be either 'PRE' or 'POST'.")
endif()
