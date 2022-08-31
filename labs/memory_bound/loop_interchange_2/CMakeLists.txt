cmake_minimum_required(VERSION 3.3)

project(lab)

set(VALIDATE_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/pexels-pixabay-434334.pbm" "${CMAKE_CURRENT_SOURCE_DIR}/output-golden.pgm")
set(LAB_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/pexels-pixabay-434334.pbm" "output.pgm")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
