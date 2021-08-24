# Get started

Watch the warmup video:

[<img src="https://drive.google.com/uc?export=view&id=1AbuZJdfc-BbpNLdxZukMILs2l5_HBH32" width="50%">](https://www.youtube.com/watch?v=jFRwAcIoLgQ&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

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

**IMPORTANT:** Send a request to be added as a collaborator to this Github repo. Otherwise, you won't be able to push your private branch[es]. Send your github handle to dendibakh@gmail.com with the topic "[PerfNinjaAccessRequest]". Do not fork the repo and submit a pull request with your solution, the CI job won't be triggered.

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
