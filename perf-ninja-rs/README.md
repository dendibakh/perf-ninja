# Rust labs for Performance Ninja Class

Rust port of the exercises in https://github.com/dendibakh/perf-ninja

You will need to watch the videos at the parent project, that's the course. To do the course in Rust, use this code instead of the parent C++ code.

I recommend reading Denis' free ebook [Performance Analysis and Tuning on Modern CPUs](https://book.easyperf.net/perf_book) as you do the course. Things can get a little confusing otherwise, and the book all by itself is excellent; real practical performance tuning advice from an expert.

## Lab assignments

* Core Bound:
  * [Vectorization 1](labs/core_bound/vectorization_1)
  * [Vectorization 2](labs/core_bound/vectorization_2)
  * [Function Inlining](labs/core_bound/function_inlining_1)
  * [Dependency Chains 1](labs/core_bound/dep_chains_1)
  * [Compiler Intrinsics 1](labs/core_bound/compiler_intrinsics_1)
  * [Compiler Intrinsics 2](labs/core_bound/compiler_intrinsics_2)
* Memory Bound:
  * [Data Packing](labs/memory_bound/data_packing)
  * [Loop Interchange 1](labs/memory_bound/loop_interchange_1): Rust version does not appear to be memory bound, see the README.
  * [Loop Interchange 2](labs/memory_bound/loop_interchange_2): Rust version does not appear to be memory bound, see the README.
  * [Loop Tiling](labs/memory_bound/loop_tiling_1)
  * [SW memory prefetching](labs/memory_bound/swmem_prefetch_1)
  * [False Sharing](labs/memory_bound/false_sharing_1)
  * [Huge Pages](labs/memory_bound/huge_pages_1)
* Bad Speculation:
  * [Conditional Store](labs/bad_speculation/conditional_store_1)
  * [Replacing Branches With Lookup Tables](labs/bad_speculation/lookup_tables_1)
  * [Rust Virtual Calls](labs/bad_speculation/virtual_call_mispredict)
* Misc:
  * [Warmup](labs/misc/warmup)
  * LTO: TODO
  * PGO: TODO
  * [Optimize IO](labs/misc/io_opt1)

The two Loop Interchange labs do not match their C++ version. They are probably not an accurate port and need changing.

These two labs match the bottlenecks of their C++ versions (under Clang 14), but have different bottlenecks than indicated.
 - Core Bound / Vectorization 1: Try debug mode, that has the correct bottleneck.
 - Memory Bound / SW memory prefetching: Not memory bound, bottleneck seems to be branch prediction.

Aside from those differences, the Rust code should serve you well in your studies to become a performance ninja!

## Setup

You need:
 - [Rust](https://www.rust-lang.org/tools/install) and switch to [nightly](https://rust-lang.github.io/rustup/concepts/channels.html) release.
 - The videos from the parent project: https://github.com/dendibakh/perf-ninja
 - [pmu-tools](https://github.com/andikleen/pmu-tools) to do the investigation.

## Layout

Each lab is a cargo project. In brackets are the mappings to the C++ version.

 - `src/lib.rs`: The code you need to optimize (solution.cpp, solution.h, init.cpp)
 - `src/tests.rs`: A unit test (validate.cpp) to check your code still works.
 - `benches/bench_<crate>.rs`: The benchmark (bench.cpp). This will tell you when you have made src/lib.rs:solution faster.

You will only need to touch the code in `lib.rs`. The unit test and the benchmark both call that code. The benchmark uses [criterion](https://docs.rs/criterion/latest/criterion/) to produce accurate numbers.

## Work loop

 1. `cargo bench`: How fast is it now?
 1. Improve the code in `lib.rs`.
 1. `cargo test --release`: Is it still correct?
 1. Goto 1.

### Better benchmarks

Criterion (which `cargo bench` is using) does statistical benchmarking, but even with that I get a lot of variance between runs. We can do much better:

 1. Download [runperf](https://gist.github.com/grahamking/9c8c91b871843a9a6ce2bec428b8f48d). This adjusts a bunch of things on Linux to provide repeatable, reliable benchmarks.
 1. Find the benchmark binary. `cargo bench` builds it as `target/release/deps/bench_<crate>_<hash>`.
 1. Run it directly: `runperf <benchmark_binary> --bench`. You should get the same results every time.

### Find bottlenecks

The videos often walk through this part. Profile the benchmark binary (in `target/release/deps/`). We need to disable criterion's overhead by passing `--profile-time <seconds>`. We always need to pass `--bench` to a Criterion benchmark binary. Use `runperf` (see above) for reliable results.

Examples:
   - `runperf perf stat ./target/release/deps/bench_<crate>_<hash> --bench --profile-time 5`
   - `runperf perf record <binary> --bench --profile-time 5` then `perf report -Mintel`.
   - `runperf ~/src/pmu-tools/toplev --core S0-C0,S0-C1 -l1 -v --no-desc <binary> --bench --profile-time 5` (then try with `-l2` instead of `-l1`)

## Misc / Tips

Optimize Rust for your CPU, and include frame pointers: `export RUSTFLAGS="-Ctarget-cpu=native -Cforce-frame-pointers=yes"`.

Have `perf report` display the call graph: `perf record --call-graph fp <prog>`. You need to build with `force-frame-pointers` (above in RUSTFLAGS).

Show assembly: `objdump -Mintel -S -d target/release/deps/bench_vectorization_2 | rustfilt`.
 - `rustfilt` de-mangles Rust symbols: `cargo install rustfilt`
 - `-S` includes source code in the output

By default `perf record` uses the `cycles` events (number of CPU cycles). If you want to dig into a specific event provide that directly to perf:
 - Branch misses (bad speculation): `runperf perf record --call-graph fp --event=branch-misses:P <prog>`
 - Main memory load (backend bound): `--event=cycle_activity.stalls_l3_miss:P` (An L3 cache miss means we have to go to main memory)

The `:P` denotes a [Precise Event](https://www.intel.com/content/www/us/en/develop/documentation/vtune-help/top/analyze-performance/custom-analysis/custom-analysis-options/hardware-event-list/precise-events.html).

`runperf` restricts execution to two cores and the `toplev` command above watches both those cores. The hope is that one core gets the tool (`toplev`, `perf`, etc) and the other core gets the program you're testing, and they both run without context switches (Linux tries to avoid moving programs between cores if possible). The downside is that it's not obvious which core your program ran on, and `toplev` output includes both. To simplify, edit `runperf`, replace `taskset -c 0,1 sudo nice -n -5 runuser -u $USERNAME -- $@` with `taskset -c 1 sudo nice -n -5 runuser -u $USERNAME -- $@` (ask taskset to only use core 1) and change the `toplev` command to `--core S0-C1` (only watch Socket 0, Core 1).

## Notes on the port

Best effort was made to keep the code as close to the C++ original as possible. That meant resisting iterator chaining, using C++ names (e.g. `ClassA`), and even sometimes ignoring `clippy`. The hope is that this makes it easier to follow along with the original videos.

## Thanks

Thanks to my employer Dropbox for supporting this project during Hack Week 2022.

If this course is useful to you please consider supporting the parent project's Patreon or GitHub Sponsors.

## License

Original problems and ideas Copyright © 2021 by Denis Bakhvalov under Creative Commons license (CC BY 4.0).
Rust port Copyright © 2022 by Graham King under Creative Commons license (CC BY 4.0).

