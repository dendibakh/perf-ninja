## Set up environment on Linux

1. Install Clang 17 using the instructions from [apt.llvm.org](https://apt.llvm.org/):

    ```
    wget https://apt.llvm.org/llvm.sh
    chmod +x llvm.sh
    sudo ./llvm.sh 17 all
    ```

2. Install CMake and Ninja:

    ```
    sudo apt install cmake ninja-build
    ```

3. Build the pinned Google Benchmark dependency inside the repository:

    ```
    ./pn bootstrap
    ```

    This does not install Google Benchmark system-wide. The version and commit
    are recorded in `tools/google-benchmark.lock.json`, and the ignored checkout
    is stored under `tools/benchmark`.

4. Optionally expose the repository command on your user-local `PATH`, then
   validate a lab:

    ```
    mkdir -p "$HOME/.local/bin"
    ln -s "$PWD/pn" "$HOME/.local/bin/pn"
    cd labs/misc/warmup
    pn validate
    ```

    The command invokes `clang-17` and `clang++-17` directly. It does not modify
    system compiler alternatives or global CMake configuration.

    Without the optional symlink, stay at the repository root and run:

    ```
    ./pn validate labs/misc/warmup
    ```

5. For more stable measurements, set the CPU frequency scaling governor to
   `performance` before benchmarking:

    ```
    sudo cpupower frequency-set --governor performance
    ```

    The `pn` command reports the active governor but never changes it
    automatically.

6. Build, validate, benchmark, compare, or submit any lab from its source
   directory:

    ```
    pn build
    pn validate
    pn bench
    pn compare
    pn submit --dry-run
    pn submit
    ```

    You can also pass a lab path when running from elsewhere, for example
    `pn compare labs/misc/warmup`. Submission stages only the selected lab and
    watches the official CI after a normal non-force push. Run `pn ci --watch`
    to reattach to CI for the current commit and branch.
