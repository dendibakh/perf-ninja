#!/bin/bash

CALLING_PATH=$(pwd)
cd "${CALLING_PATH}"

cmake -E make_directory build
cd build

if [[ "$1" == "release" ]]; then
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-18 -DCMAKE_CXX_COMPILER=clang++ ..
    cmake --build . --config Release --parallel 8
    cmake --build . --target validateLab
    cmake --build . --target benchmarkLab
else
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang-18 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_FLAGS="-O0" -DCMAKE_CXX_FLAGS="-O0" ..
    cmake --build . --config Debug --parallel 8
    cmake --build . --target validateLab
    cmake --build . --target benchmarkLab
fi