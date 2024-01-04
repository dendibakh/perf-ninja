## Set up environment on Mac OS

**Mac support is still experimental. Submit bugs if you experience issues.**

1. Run terminal.

2. Install homebrew (if haven't already) following instructions from [here](https://brew.sh):

    ```
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile\n
    ```

2. Install clang-17 compiler (make sure that the version is not older than 15, otherwise it will not compile):

    ```
    brew install llvm@17
    export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
    export CC=clang
    export CXX=clang++
    ```

    Consider making environment changes permanent:

    ```
    echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
    echo 'export CC=clang' >> ~/.zshrc
    echo 'export CXX=clang++' >> ~/.zshrc
    ```

5. Build release version of google [benchmark library](https://github.com/google/benchmark#installation). It doesn't matter which compiler you use to build it. Install google benchmark library with:
    
    ```
    git clone https://github.com/google/benchmark.git
    cd benchmark
    git clone https://github.com/google/googletest.git
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . --config Release --target install
    ```

6. Go to any lab and check if local lab builds are working. You can find the CMake commands [here](GetStarted.md#how-to-build-lab-assignments). 

7. (Optional) Install [ninja](https://github.com/ninja-build).
    
    ```
    $ brew install ninja
    ```
    
    You can use it to build labs by passing `-G Ninja` to the CMake invocation.
