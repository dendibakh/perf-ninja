cmake_minimum_required(VERSION 3.3)

project(lab)

include_directories(lua)
file(GLOB EXT_LAB_srcs lua/*.c)
file(GLOB EXT_VALIDATE_srcs lua/*.c)

set(VALIDATE_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/reference_output.txt")

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)

set(LLVM_PROFDATA "/usr/lib/llvm-17/bin/llvm-profdata")

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
  # Copy "lab" target and add instrumentation to it
  add_executable(lab_pgo)
  target_sources(lab_pgo PRIVATE $<TARGET_PROPERTY:lab,SOURCES>)
  target_include_directories(lab_pgo PRIVATE $<TARGET_PROPERTY:lab,INCLUDE_DIRECTORIES>)
  target_compile_options(lab_pgo PRIVATE $<LIST:REMOVE_ITEM,$<TARGET_PROPERTY:lab,COMPILE_OPTIONS>,-fprofile-instr-use=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata>)
  target_compile_options(lab_pgo PRIVATE "-fprofile-instr-generate=${CMAKE_CURRENT_BINARY_DIR}/default.profraw")
  target_link_libraries(lab_pgo PRIVATE $<TARGET_PROPERTY:lab,LINK_LIBRARIES>)
  target_link_options(lab_pgo PRIVATE $<LIST:REMOVE_ITEM,$<TARGET_PROPERTY:lab,COMPILE_OPTIONS>,-fprofile-instr-use=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata>)
  target_link_options(lab_pgo PRIVATE "-fprofile-instr-generate=${CMAKE_CURRENT_BINARY_DIR}/default.profraw")

  # Custom target to automate the profile collection process
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_CURRENT_BINARY_DIR}/default.profraw ${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/lab_pgo
    COMMAND ${LLVM_PROFDATA} merge -output=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata ${CMAKE_CURRENT_BINARY_DIR}/default.profraw
    DEPENDS lab_pgo
  )
  add_custom_target(collect_pgo_profile DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata)

  target_compile_options(lab PRIVATE -fprofile-instr-use=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata)
  target_link_options(lab PRIVATE -fprofile-instr-use=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata)
  add_dependencies(lab collect_pgo_profile)
  target_compile_options(validate PRIVATE -fprofile-instr-use=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata)
  target_link_options(validate PRIVATE -fprofile-instr-use=${CMAKE_CURRENT_BINARY_DIR}/pgo.profdata)
  add_dependencies(validate collect_pgo_profile)
endif()
