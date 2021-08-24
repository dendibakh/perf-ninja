<p align="center"> <img src="/logo.jpg" width=200> </p>

# Performance Ninja Class

This is an online course where you can learn to find and fix low-level performance issues, for example CPU cache misses and branch mispredictions. It's all about practice. So we offer you this course in a form of lab assignments and youtube videos. You will spend at least 90% of the time analyzing performance of the code and trying to improve it.

[<img src="https://drive.google.com/uc?export=view&id=1pYZEkSV3fiLo04b0UdJzHoEhLkhc6T09" width="50%">](https://www.youtube.com/watch?v=2tzdkC6IDbo&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Each lab assignment focuses on a specific performance problem and can take anywhere from 30 mins up to 4 hours depending on your background and the complexity of the lab assignment itself. Once you're done improving the code, you can submit your solution to Github for automated benchmarking and verification.

Before you start working on lab assignments, make sure you read [Get Started page](GetStarted.md) and watch the [warmup video](https://youtu.be/jFRwAcIoLgQ). Join our [discord channel](https://discord.gg/8h6gEzgMDY) to collaborate with others.

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

## Support the project

Performance Ninja is in a very much work-in-progress state. We will be adding new lab assignments and videos! The course is free by default, but we ask you to support us on [Github Sponsors](https://github.com/sponsors/dendibakh), [Patreon](https://www.patreon.com/dendibakh) or [PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=TBM3NW8TKTT34&currency_code=USD&source=url). Your sponsorship will speed up adding new lab assignments.

## Contributing

We warmly welcome contributions! Here is how you can help the project:
* Propose/Implement new lab assignment.
* Donate your idle machine to run CI jobs. We are looking to increase diversity of the platforms on which we test your submissions.

Please write to dendibakh@gmail.com with suggestions.

Copyright © 2021 by Denis Bakhvalov under Creative Commons license (CC BY 4.0).
