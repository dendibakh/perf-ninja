cmake_minimum_required(VERSION 3.3)

project(lab)

set(LAB_DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(BIRD_PATH "${LAB_DATA_PATH}/bird.pgm")
set(COINS_PATH "${LAB_DATA_PATH}/coins.pgm")
set(PEPPER_PATH "${LAB_DATA_PATH}/pepper.pgm")
set(PIXABAY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../loop_interchange_2/pexels-pixabay-434334.pbm")
configure_file(DataPaths.h.in "${CMAKE_CURRENT_SOURCE_DIR}/DataPaths.h")

set(PEPPER_REF_PATH "${LAB_DATA_PATH}/pepper-binary-ref.pgm")
set(VALIDATE_ARGS "${PEPPER_PATH}" "${PEPPER_REF_PATH}")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)