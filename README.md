<p align="center"> <img src="/logo.jpg" width=200> </p>

# Performance Ninja Class

This is an online course where you can learn and master the skill of Performance Analysis and Tuning.

Work in progress...

TODO: add introductory video

## Get started

This course consits of lab asignments that are divided in sections. Each section has an introductory video covering a specific class of optimizations.

Every lab assignment has the following:
* Video that introduces a particular transformation.
* Baseline version of a workload that has a particular performance bottleneck in it. You need to find it and fix the source code accordingly.
* Summary video that explains the solution for the lab.

We encourage you to work on the lab assignment first, without watching the summary video.

Every lab can be built and ran using the following commands:
```
cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
```
When you push changes to your private branch, it will automatically trigger CI benchmarking job. More details about it at the bottom of the page.

## Lab assignments

* Memory Bound:
  * [Data Packing](labs/memory_bound/data_packing)
  * [Loop Interchange 1](labs/memory_bound/loop_interchange_1)
  * [Loop Interchange 2](labs/memory_bound/loop_interchange_2)
* Core Bound:
  * [Vectorization 1](labs/core_bound/vectorization_1)
  * [Function Inlining 1](labs/core_bound/function_inlining_1)
  * [Compiler Intrinsics 1](labs/core_bound/compiler_intrinsics_1)
* Bad Speculation:
* CPU Frontend Bound:
* Data-Driven optimizations:
* Misc:
  * [Warmup](labs/misc/warmup)

## Project dependencies

To build labs in this video course you need to install the following dependencies:
* CMake 3.13
* [Google benchmark](https://github.com/google/benchmark), you can also use the scripts in the [tools](tools) directory.

## Target platform

You are free to work on whatever platform you have at your disposal. However, the CI machine, which is used to run your submissions has the following configuration:
* Intel(R) Core(TM) i5-8259U CPU @ 2.30GHz, 6MB L3-cache
* 8 GB RAM
* Ubuntu 20.04
* Clang C++ compiler, version 12.0

## Submission guidelines:

Push your submissions into your own branch[es]. CI job will be triggered every time you push changes to your remote Github branch. For now, we use a self-hosted runner, which is configured specifically for benchmarking purposes.

By default, CI will detect which lab was modified in the last commit and will only benchmark affected asignment. If you make changes to more than one lab, CI job will benchmark all the labs. You can also force benchmarking all the labs if you add `[CheckAll]` in the commit message.

In case all the labs were benchmarked, summary will be provided at the end, e.g.:

```
Lab Assignments Summary:
  memory_bound:
    data_packing: Passed
    sequential_accesses: Failed: not fast enough
  core_bound:
    function_inlining: Failed: build error
  misc:
    warmup: Skipped
```

## Contributing

We warmly welcome contributions! Here is how you can help the project:
* Propose/Implement new lab assignment.
* Donate your idle machine to run CI jobs. We are looking to increase diversity of the platforms on which we test your submissions.

Please write to dendibakh@gmail.com with suggestions.

Copyright © 2021 by Denis Bakhvalov under Creative Commons license (CC BY 4.0).
