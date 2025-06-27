# Warmup

This is a warmup lab assignment.

[<img src="https://img.youtube.com/vi/jFRwAcIoLgQ/maxresdefault.jpg" width="30%">](https://youtu.be/jFRwAcIoLgQ)

## Building the Lab
```
cmake -E make_directory build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release -Dbenchmark_DIR=/home/jackiedong/Documents/perf-ninja/benchmark/build ..
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
```

## Build with debug information
```
cmake -E make_directory build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Debug -Dbenchmark_DIR=/home/jackiedong/Documents/perf-ninja/benchmark/build -DCMAKE_C_FLAGS=-g -DCMAKE_CXX_FLAGS=-g ..
cmake --build . --config Debug --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
```