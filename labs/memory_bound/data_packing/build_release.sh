#!/usr/bin/env bash

export CC="clang"
export CXX="clang++"

rm -rf build

cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
