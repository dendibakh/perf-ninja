# Can I do this?
cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release .. -DCMAKE_C_COMPILER=clang-17 -DCMAKE_CXX_COMPILER=clang++
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
