cmake_minimum_required(VERSION 3.3)

project(lab)

function(write_random_string path len)
  if(NOT EXISTS ${path})
    message("Generating file with random contents of length ${len}, this may take a few seconds...")
    string(RANDOM LENGTH ${len} contents)
    file(WRITE "${path}" "${contents}")
  endif()
endfunction()

set(LAB_DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(SMALL_DATA_PATH "${LAB_DATA_PATH}/small.data")
set(MEDIUM_DATA_PATH "${LAB_DATA_PATH}/medium.data")
set(LARGE_DATA_PATH "${LAB_DATA_PATH}/large.data")
file(MAKE_DIRECTORY "${LAB_DATA_PATH}")
write_random_string("${SMALL_DATA_PATH}"  4095)
write_random_string("${MEDIUM_DATA_PATH}" 2097151)
write_random_string("${LARGE_DATA_PATH}"  268435455)
configure_file(DataPaths.h.in "${CMAKE_CURRENT_SOURCE_DIR}/DataPaths.h")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
