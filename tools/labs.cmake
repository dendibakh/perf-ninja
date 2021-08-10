# https://cmake.org/documentation/

# Check usage of 'build' subdirectory
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message("CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
  message("Please always use the 'build' subdirectory:")
  message(FATAL_ERROR "  git clean -dfx & mkdir build & cd build & cmake -DCMAKE_BUILD_TYPE=Release ..")
endif()

# Just use the variable
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message("Please consider: cmake -DCMAKE_BUILD_TYPE=Release ..")
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# Set compiler options
if(NOT MSVC)
  set(CMAKE_C_FLAGS "-O3 -ffast-math -march=native ${CMAKE_C_FLAGS}")
else()
  set(CMAKE_C_FLAGS "/O2 /fp:fast /arch:AVX2 ${CMAKE_C_FLAGS}")

  # Set stack size as on Linux: 2MB on 32-bit, 8MB on 64-bit
  math(EXPR stack_size "${CMAKE_SIZEOF_VOID_P}*${CMAKE_SIZEOF_VOID_P}*128*1024")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${stack_size}")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

# https://github.com/google/benchmark
set(BENCHMARK_FOLDER "${CMAKE_CURRENT_LIST_DIR}/benchmark")
find_library(BENCHMARK_LIBRARY NAMES benchmark PATHS "${BENCHMARK_FOLDER}/build/src/Release" "${BENCHMARK_FOLDER}/build/src" REQUIRED)

# Find source files
file(GLOB srcs *.c *.h *.cpp *.hpp *.cxx *.hxx *.inl)
list(FILTER srcs EXCLUDE REGEX ".*bench.cpp$")
list(FILTER srcs EXCLUDE REGEX ".*validate.cpp$")

# Add main targets
add_executable(lab bench.cpp ${srcs})
add_executable(validate validate.cpp ${srcs})

# Add path to a local benchmark library
if(EXISTS "${BENCHMARK_FOLDER}/include")
  target_include_directories(lab BEFORE PRIVATE "${BENCHMARK_FOLDER}/include")
  target_include_directories(validate BEFORE PRIVATE "${BENCHMARK_FOLDER}/include")
endif()

# Check optional arguments
if(NOT DEFINED CI)
  set(CI OFF)
endif()
if(NOT DEFINED VALIDATE_ARGS)
  set(VALIDATE_ARGS "")
endif()
if(NOT DEFINED LAB_ARGS)
  set(LAB_ARGS "")
endif()
set(LAB_BENCHMARK_ARGS --benchmark_min_time=1 --benchmark_out_format=json --benchmark_out=result.json)

if(CI)
  # Add CI targets without dependencies
  get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(isMultiConfig)
    add_custom_target(validateLab
      COMMAND ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/validate ${VALIDATE_ARGS}
      VERBATIM)

    add_custom_target(benchmarkLab
      COMMAND ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lab ${LAB_ARGS} ${LAB_BENCHMARK_ARGS}
      VERBATIM)
  else()
    add_custom_target(validateLab
      COMMAND ${PROJECT_BINARY_DIR}/validate ${VALIDATE_ARGS}
      VERBATIM)

    add_custom_target(benchmarkLab
      COMMAND ${PROJECT_BINARY_DIR}/lab ${LAB_ARGS} ${LAB_BENCHMARK_ARGS}
      VERBATIM)
  endif()
else()
  # Add robust execution targets
  add_custom_target(validateLab
    COMMAND validate ${VALIDATE_ARGS}
    VERBATIM)

  add_custom_target(benchmarkLab
    COMMAND lab ${LAB_ARGS} ${LAB_BENCHMARK_ARGS}
    VERBATIM)
endif()

# Other settings
if(NOT MSVC)
  target_link_libraries(lab ${BENCHMARK_LIBRARY} pthread m)
  target_link_libraries(validate ${BENCHMARK_LIBRARY} pthread m)

  # MinGW
  if(MINGW)
    target_link_libraries(lab shlwapi)
    target_link_libraries(validate shlwapi)
  endif()
else()
  find_library(BENCHMARK_LIBRARYD NAMES benchmark PATHS "${BENCHMARK_FOLDER}/build/src/Debug")

  target_link_libraries(lab Shlwapi.lib optimized ${BENCHMARK_LIBRARY} debug ${BENCHMARK_LIBRARYD})
  target_link_libraries(validate Shlwapi.lib optimized ${BENCHMARK_LIBRARY} debug ${BENCHMARK_LIBRARYD})

  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT lab)

  string(REPLACE ";" " " LAB_ARGS_STR "${LAB_ARGS}")
  string(REPLACE ";" " " VALIDATE_ARGS_STR "${VALIDATE_ARGS}")

  # Since CMake 3.13.0
  set_property(TARGET lab PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "${LAB_ARGS_STR}")
  set_property(TARGET validate PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "${VALIDATE_ARGS_STR}")

  # Hide helper projects
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  set_target_properties(validateLab PROPERTIES FOLDER CI)
  set_target_properties(benchmarkLab PROPERTIES FOLDER CI)
endif()
