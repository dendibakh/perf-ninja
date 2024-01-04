## Set up environment on Windows

1. Run powershell.

2. Install [ninja](https://github.com/ninja-build/ninja/releases). 
    
    Add it to the PATH. For example:
    ```
    $ENV:PATH="$ENV:PATH;C:\Program Files\ninja"
    ```
3. Download clang-17 compiler from [here](https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.1) (LLVM-17.0.1-win64.exe) and install it. Select "add LLVM to the PATH" while installing.

4. Build release version of google [benchmark library](https://github.com/google/benchmark#installation). It doesn't matter which compiler you use to build it. Install google benchmark library with:
    ```
    cmake --build "build" --config Release --target install
    ```
    Add google benchmark library to PATH
    ```
    $ENV:PATH="$ENV:PATH;C:\Program Files (x86)\benchmark\lib"
    ```
5. Go to any lab and check if local lab builds are working. You can find the CMake commands [here](GetStarted.md#how-to-build-lab-assignments), but note that you need to add `-G Ninja` to the CMake invocation.

6. If everything works as expected, you can set environment variables permanently (run as Administrator):
    ```
    # be carefull, back up your PATH
    setx /M PATH "$($env:path);C:\Program Files\ninja"
    setx /M PATH "$($env:path);C:\Program Files (x86)\benchmark\lib"
    ```