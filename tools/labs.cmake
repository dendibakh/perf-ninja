# https://cmake.org/documentation/

# Check usage of 'build' subdirectory
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message("CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
  message("Please always use the 'build' subdirectory:")
  if(MSVC)
    message(FATAL_ERROR "git clean -dfx & cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -T ClangCL")
  else()
    message(FATAL_ERROR "git clean -dfx & mkdir build & cd build & cmake -DCMAKE_BUILD_TYPE=Release ..")
  endif()
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
  include("${CMAKE_CURRENT_LIST_DIR}/msvc_simd_isa.cmake")
  if(SUPPORT_MSVC_AVX512)
    set(MSVC_SIMD_FLAGS "/arch:AVX512")
  elseif(SUPPORT_MSVC_AVX2)
    set(MSVC_SIMD_FLAGS "/arch:AVX2")
  elseif(SUPPORT_MSVC_AVX)
    set(MSVC_SIMD_FLAGS "/arch:AVX")
  else()
    set(MSVC_SIMD_FLAGS "")
  endif()
  set(CMAKE_C_FLAGS "/O2 /fp:fast ${MSVC_SIMD_FLAGS} ${CMAKE_C_FLAGS}")
endif()

# Set Windows stack size as on Linux: 2MB on 32-bit, 8MB on 64-bit
if (WIN32)
  math(EXPR stack_size "${CMAKE_SIZEOF_VOID_P}*${CMAKE_SIZEOF_VOID_P}*128*1024")
  if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${stack_size}")
  else()
    # compiling with clang + lld
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker /stack:${stack_size}")
  endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

# https://github.com/google/benchmark
find_package(benchmark PATHS "${CMAKE_CURRENT_LIST_DIR}/benchmark/build" REQUIRED)
set(BENCHMARK_LIBRARY "benchmark::benchmark")

# Find source files
file(GLOB srcs *.c *.h *.cpp *.hpp *.cxx *.hxx *.inl)
list(FILTER srcs EXCLUDE REGEX ".*bench.cpp$")
list(FILTER srcs EXCLUDE REGEX ".*validate.cpp$")

# Add main targets
add_executable(lab bench.cpp ${srcs} ${EXT_LAB_srcs})
add_executable(validate validate.cpp ${srcs} ${EXT_VALIDATE_srcs})

# Add path to a local benchmark library
if(EXISTS "${BENCHMARK_FOLDER}/include")
  target_include_directories(lab BEFORE PRIVATE "${BENCHMARK_FOLDER}/include")
  target_include_directories(validate BEFORE PRIVATE "${BENCHMARK_FOLDER}/include")
endif()
if(EXISTS "${BENCHMARK_FOLDER}/build/include")
  target_include_directories(lab BEFORE PRIVATE "${BENCHMARK_FOLDER}/build/include")
  target_include_directories(validate BEFORE PRIVATE "${BENCHMARK_FOLDER}/build/include")
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

if("${BENCHMARK_MIN_TIME}" STREQUAL "")
  set(BENCHMARK_MIN_TIME "2s")
endif()
set(LAB_BENCHMARK_ARGS --benchmark_min_time=${BENCHMARK_MIN_TIME} --benchmark_out_format=json --benchmark_out=result.json)

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
  if (WIN32)
    target_link_libraries(lab ${BENCHMARK_LIBRARY} shlwapi)
    target_link_libraries(validate ${BENCHMARK_LIBRARY} shlwapi)
  else()
    target_link_libraries(lab ${BENCHMARK_LIBRARY} pthread m)
    target_link_libraries(validate ${BENCHMARK_LIBRARY} pthread m)
  endif()

  # MinGW
  if(MINGW)
    target_link_libraries(lab shlwapi)
    target_link_libraries(validate shlwapi)
  endif()
else()
  target_link_libraries(lab Shlwapi.lib ${BENCHMARK_LIBRARY})
  target_link_libraries(validate Shlwapi.lib ${BENCHMARK_LIBRARY})

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
