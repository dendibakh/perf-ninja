## Set up environment on Linux

1. Run terminal.

2. Install clang-17 compiler using instructions from [here](https://apt.llvm.org/):

    ```
    wget https://apt.llvm.org/llvm.sh
    chmod +x llvm.sh
    sudo ./llvm.sh 17 all
    ```

3. Build release version of google [benchmark library](https://github.com/google/benchmark#installation). It doesn't matter which compiler you use to build it. Install google benchmark library with:
    ```
    cmake --build "build" --config Release --target install
    ```

4. Enable clang-17 compiler for building labs. If you want to make clang-17 to be the default on a system do the following:
    ```
    sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-17 30
    sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-17 30
    ```

    If you don't want to make it a default, you can pass `-DCMAKE_C_COMPILER=clang-17 -DCMAKE_CXX_COMPILER=clang++-17` to the CMake.

5. Go to any lab and check if local lab builds are working. You can find the CMake commands [here](GetStarted.md#how-to-build-lab-assignments). 

6. Set the frequency scaling governor to `performance`.
    ```
    sudo cpupower frequency-set --governor performance
    ```

7. (Optional) Install [ninja](https://github.com/ninja-build).
    
    ```
    $ sudo apt install ninja-build
    ```
    
    You can use it to build labs by passing `-G Ninja` to the CMake invocation.
