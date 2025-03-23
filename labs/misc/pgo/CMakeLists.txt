cmake_minimum_required(VERSION 3.3)

project(lab)

include_directories(lua)
file(GLOB EXT_LAB_srcs lua/*.c)
file(GLOB EXT_VALIDATE_srcs lua/*.c)

set(VALIDATE_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/reference_output.txt")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)

if(NOT DEFINED LLVM_PROFDATA)
  # file(REAL_PATH ${CMAKE_C_COMPILER} CLANG_DIR)
  get_filename_component(CLANG_PATH ${CMAKE_C_COMPILER} REALPATH)
  get_filename_component(CLANG_PATH ${CLANG_PATH} DIRECTORY)
  set(LLVM_PROFDATA "${CLANG_PATH}/llvm-profdata")
  message(STATUS "llvm-profdata: ${LLVM_PROFDATA}")
endif()
set(PGO_GENERATE -fprofile-instr-generate=${CMAKE_CURRENT_BINARY_DIR}/default.profraw)
set(PGO_USE -fprofile-instr-use=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
  # Copy "lab" target and add instrumentation to it
  add_executable(lab_pgo)
  target_sources(lab_pgo PRIVATE $<TARGET_PROPERTY:lab,SOURCES>)
  target_include_directories(lab_pgo PRIVATE $<TARGET_PROPERTY:lab,INCLUDE_DIRECTORIES>)
  target_compile_options(lab_pgo PRIVATE ${PGO_GENERATE})
  target_link_libraries(lab_pgo PRIVATE $<TARGET_PROPERTY:lab,LINK_LIBRARIES>)
  target_link_options(lab_pgo PRIVATE ${PGO_GENERATE})

  # Custom target to automate the profile collection process
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_CURRENT_BINARY_DIR}/default.profraw ${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/lab_pgo
    COMMAND ${LLVM_PROFDATA} merge -output=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata ${CMAKE_CURRENT_BINARY_DIR}/default.profraw
    DEPENDS lab_pgo
  )
  add_custom_target(collect_pgo_profile DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata)

  target_compile_options(lab PRIVATE ${PGO_USE})
  target_link_options(lab PRIVATE ${PGO_USE})
  add_dependencies(lab collect_pgo_profile)
  target_compile_options(validate PRIVATE ${PGO_USE})
  target_link_options(validate PRIVATE ${PGO_USE})
  add_dependencies(validate collect_pgo_profile)
endif()
