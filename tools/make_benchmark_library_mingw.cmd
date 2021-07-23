@echo See https://github.com/google/benchmark

@rem Check out the library.
git clone https://github.com/google/benchmark.git

@rem Benchmark requires Google Test as a dependency. Add the source tree as a subdirectory.
git clone https://github.com/google/googletest.git benchmark/googletest

@rem Go to the library root directory
cd benchmark
@rem Make a build directory to place the build output.
cmake -E make_directory "build"

@rem Generate build system files with cmake.
cmake -E chdir "build" cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE_GTEST_TESTS=OFF ../
@rem Build the library.
cmake --build "build" --config Release --parallel 4
