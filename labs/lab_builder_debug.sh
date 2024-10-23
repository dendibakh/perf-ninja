cmake -E make_directory build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-g" -DCMAKE_CXX_FLAGS="-g" ..
cmake --build . --config Debug --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab