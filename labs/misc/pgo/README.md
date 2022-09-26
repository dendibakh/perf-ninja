[<img src="../../../img/PGO.png">](https://www.youtube.com/watch?v=ERqFtOZ61AA&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Profile Guided Optimizations (PGO) are a set of transformations in most optimizing compilers that can adjust their algorithms based on the profiling data. Sometimes in literature, one can find the term Feedback Directed Optimizations (FDO), which essentially refers to the same thing. The quality and relevance of the profiling data has a critical impact on performance since compiler will be "trained" using that data and use it for generating machine code. Profiling data helps compiler improve its inlining decisions, code placement, register allocation, and more. It is not uncommon to see real workloads performance increase by up to 15% from using PGO.

In this lab assignment you'll need to improve the runtime of processing a set of specific LUA scripts. There is a complete code of LUA interpreter that is being built from sources. Remember, the goal is to generate the LUA interpreter that will be optimized specific set of scripts that we provide. It's quite possible that you may see performance drop when you will run othe LUA scripts (be careful with it when using in production).

Suggested workflow for this exercise:
1. Profile the benchmark and examine the bottleneck.
2. Collect profiling data, take a look at its contents ([llvm-profdata -show](https://llvm.org/docs/CommandGuide/llvm-profdata.html#profdata-show))
3. Recompile with using profiling data collected and measure the speedup.
4. Collect the profile again to find out what changes were done by a compiler that led to perf speedup.
5. Optional: check the speedup of using PGO with other compilers (GCC, MSVC, Intel OneAPI).

# Submission

Hint: to solve this lab assignment you need to generate profiling data for the target compiler (Clang-12) and add a proper option to use this profiling data.

Expected speedup: at least 5%.

# Credits

- The code for the LUA interpreter as well as `bench` lua scripts and their inputs were taken from the llvm [test-suite](https://github.com/llvm/llvm-test-suite/tree/main/MultiSource/Applications/lua) with minor changes. Some unused files were removed.
- Lua script `md5.lua` is taken from https://github.com/kikito/md5.lua (MIT license).

Co-authored-by: @dendibakh