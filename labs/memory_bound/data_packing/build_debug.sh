#!/usr/bin/env bash

export CC="clang"
export CXX="clang++"

rm -rf build
cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. -DCMAKE_C_FLAGD='-g' -DCMAKE_CXX_FLAGD='-g'
cmake --build . --config Debug --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab