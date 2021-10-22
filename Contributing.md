# Contributing

We are looking for proposals and implementations of the new lab assignments similar to the ones we already have.

Some criterias:
- We prefer a small localized example over a big application (ideally within 1KLOC). But if something is hard to write on our own, we could reuse existing benchmark.
- We prefer real-world problems over synthesized ones. But synthesized benchmarks are welcome too.
- Performance issue in the lab assignment should be the top hotspot. I.e. the benchmark should be stressing the major performance bottleneck, not the secondary one.
- Deoptimize existing benchmarks/workloads and making a lab out of it is also fine. I.e. if you know of a case when a certain optimization *was* made in the codebase, we can use the version *before* that change was introduced.

Please *do not* submit new lab assignments as PR against this repo. Otherwise we will spoil the solution :) . I have internal repo, which we use for staging the code for new lab assignments. Write to me and I will give you access there.

We have created a template for streamlining the process of creating new lab assignments. You don't have to spend time on setting up an infrastructure and you can focus directly on the code of the assignment.

We are also looking to increase diversity of the platforms on which we test your submissions. For now, the CI runs on a dedicated Intel x86 Linux box (at Denis' home). We would gladly extend this list with ARM- and AMD-based machines. If you have one sitting idle in at your desk, consider using it to run CI jobs for this project. All you need is to install Github [runner client](https://docs.github.com/en/actions/hosting-your-own-runners/about-self-hosted-runners), which you can kill at any time.

Please write to dendibakh@gmail.com with any comments & suggestions.

## License

This project is licensed under the terms of the Creative Commons license ([CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)). By contributing to this project, you agree to the copyright terms and release your contribution under these terms.