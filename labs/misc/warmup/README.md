# Warmup

This is a warmup lab assignment.

[<img src="https://img.youtube.com/vi/jFRwAcIoLgQ/maxresdefault.jpg" width="30%">](https://youtu.be/jFRwAcIoLgQ)


cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab   [--config Release ]
cmake --build . --target benchmarkLab  [--config Release ]