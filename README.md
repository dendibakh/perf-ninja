<p align="center"> <img src="/logo.jpg" width=200> </p>

![Linux](https://github.com/dendibakh/perf-ninja/actions/workflows/CI_Linux.yml/badge.svg) ![Windows](https://github.com/dendibakh/perf-ninja/actions/workflows/CI_Win.yml/badge.svg)

# Performance Ninja Class

This is an online course where you can learn to find and fix low-level performance issues, for example CPU cache misses and branch mispredictions. It's all about practice. So we offer you this course in a form of lab assignments and youtube videos. You will spend at least 90% of the time analyzing performance of the code and trying to improve it.

[<img src="https://drive.google.com/uc?export=view&id=1pYZEkSV3fiLo04b0UdJzHoEhLkhc6T09" width="30%">](https://www.youtube.com/watch?v=2tzdkC6IDbo&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Each lab assignment focuses on a specific performance problem and can take anywhere from 30 mins up to 4 hours depending on your background and the complexity of the lab assignment itself. Once you're done improving the code, you can submit your solution to Github for automated benchmarking and verification.

Before you start working on lab assignments, make sure you read [Get Started page](GetStarted.md) and watch the [warmup video](https://youtu.be/jFRwAcIoLgQ). Join our [discord channel](https://discord.gg/8h6gEzgMDY) to collaborate with others.

Lab assignments in this project are implemented in C++. If you're more comfortable with Rust, perf-ninja was also ported to Rust [here](https://github.com/grahamking/perf-ninja-rs), thanks to @grahamking.

## Lab assignments

* [Core Bound](labs/core_bound):
  * [Vectorization 1](labs/core_bound/vectorization_1)
  * [Vectorization 2](labs/core_bound/vectorization_2)
  * [Function Inlining](labs/core_bound/function_inlining_1)
  * [Dependency Chains 1](labs/core_bound/dep_chains_1)  
  * [Compiler Intrinsics 1](labs/core_bound/compiler_intrinsics_1)
  * [Compiler Intrinsics 2](labs/core_bound/compiler_intrinsics_2)
* [Memory Bound](labs/memory_bound):
  * [Data Packing](labs/memory_bound/data_packing)
  * [Loop Interchange 1](labs/memory_bound/loop_interchange_1)
  * [Loop Interchange 2](labs/memory_bound/loop_interchange_2)
  * [Loop Tiling](labs/memory_bound/loop_tiling_1)
  * [SW memory prefetching](labs/memory_bound/swmem_prefetch_1)
  * [False Sharing](labs/memory_bound/false_sharing_1)
* [Bad Speculation](labs/bad_speculation):
  * [Conditional Store](labs/bad_speculation/conditional_store_1)
  * [Replacing Branches With Lookup Tables](labs/bad_speculation/lookup_tables_1)
  * [C++ Virtual Calls](labs/bad_speculation/virtual_call_mispredict)
* CPU Frontend Bound:
* Data-Driven optimizations:
* Misc:
  * [Warmup](labs/misc/warmup)
  * [LTO](labs/misc/lto)
  * [PGO](labs/misc/pgo)

## Support the project

Performance Ninja is in a very much work-in-progress state. We will be adding new lab assignments and videos! The course is free by default, but we ask you to support us on [Github Sponsors](https://github.com/sponsors/dendibakh), [Patreon](https://www.patreon.com/dendibakh) or [PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=TBM3NW8TKTT34&currency_code=USD&source=url). Your sponsorship will speed up adding new lab assignments.

Current sponsors:
* Pavel Davydov
* Maya Lekova (@MayaLekova)
* Aaron St. George (@AaronStGeorge)

Thanks to Mansur Mavliutov (@Mansur) for providing an AMD-based machine for running CI jobs.

Lab authors:
* Andrew Evstyukhin (@andrewevstyukhin)
* Ivica Bogosavljevic (@ibogosavljevic)
* René Rahn (@rrahn)
* Adam Folwarczny (@adamf88)
* Jakub Beránek (@Kobzol)
* Denis Bakhvalov (@dendibakh)

## Contributing

We warmly welcome contributions! See [Contributing.md](Contributing.md) for the details.

Please write to dendibakh@gmail.com with suggestions.

Copyright © 2021 by Denis Bakhvalov under Creative Commons license (CC BY 4.0).
