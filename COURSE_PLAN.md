# Performance Ninja Course Plan

This plan follows the exercise lists in [perf-book](https://github.com/dendibakh/perf-book), starting with `warmup` and then continuing through chapters 8–13. Assignments within each chapter follow the author's listing order; the book does not specify that each assignment requires completing the previous one.

Read [Get Started](GetStarted.md) before beginning. Check off assignments as you complete them.

## Warmup

The book mentions `warmup` as an example of assignment notation in [section 1.8](https://github.com/dendibakh/perf-book/blob/main/chapters/1-Introduction/1-8%20Exercises.md). This plan places it first.

- [ ] [warmup](labs/misc/warmup/README.md)

## Chapter 8. Optimizing Memory Accesses

Source: [Questions and Exercises](https://github.com/dendibakh/perf-book/blob/main/chapters/8-Optimizing-Memory-Accesses/8-8%20Questions-Exercises.md).

- [ ] [data_packing](labs/memory_bound/data_packing/README.md)
- [ ] [huge_pages_1](labs/memory_bound/huge_pages_1/README.md)
- [ ] [swmem_prefetch_1](labs/memory_bound/swmem_prefetch_1/README.md)

## Chapter 9. Optimizing Computations

Source: [Questions and Exercises](https://github.com/dendibakh/perf-book/blob/main/chapters/9-Optimizing-Computations/9-6%20Questions-Exercises.md).

- [ ] [function_inlining_1](labs/core_bound/function_inlining_1/README.md)
- [ ] [vectorization_1](labs/core_bound/vectorization_1/README.md)
- [ ] [vectorization_2](labs/core_bound/vectorization_2/README.md)
- [ ] [dep_chains_1](labs/core_bound/dep_chains_1/README.md)
- [ ] [dep_chains_2](labs/core_bound/dep_chains_2/README.md)
- [ ] [compiler_intrinsics_1](labs/core_bound/compiler_intrinsics_1/README.md)
- [ ] [compiler_intrinsics_2](labs/core_bound/compiler_intrinsics_2/README.md)
- [ ] [loop_interchange_1](labs/memory_bound/loop_interchange_1/README.md)
- [ ] [loop_interchange_2](labs/memory_bound/loop_interchange_2/README.md)
- [ ] [loop_tiling_1](labs/memory_bound/loop_tiling_1/README.md)

## Chapter 10. Optimizing Branch Prediction

Source: [Questions and Exercises](https://github.com/dendibakh/perf-book/blob/main/chapters/10-Optimizing-Branch-Prediction/10-5%20Questions-Exercises.md).

- [ ] [branches_to_cmov_1](labs/bad_speculation/branches_to_cmov_1/README.md)
- [ ] [lookup_tables_1](labs/bad_speculation/lookup_tables_1/README.md)
- [ ] [virtual_call_mispredict](labs/bad_speculation/virtual_call_mispredict/README.md)
- [ ] [conditional_store_1](labs/bad_speculation/conditional_store_1/README.md)

## Chapter 11. Machine Code Layout Optimizations

Source: [Questions and Exercises](https://github.com/dendibakh/perf-book/blob/main/chapters/11-Machine-Code-Layout-Optimizations/11-10%20Questions-Exercises.md).

- [ ] [pgo](labs/misc/pgo/README.md)
- [ ] [lto](labs/misc/lto/README.md)

## Chapter 12. Other Tuning Areas

Source: [Questions and Exercises](https://github.com/dendibakh/perf-book/blob/main/chapters/12-Other-Tuning-Areas/12-8%20Questions-Exercises.md).

- [ ] [mem_order_violation_1](labs/memory_bound/mem_order_violation_1/README.md)
- [ ] [mem_alignment_1](labs/memory_bound/mem_alignment_1/README.md)
- [ ] [io_opt1](labs/misc/io_opt1/README.md)

## Chapter 13. Optimizing Multithreaded Applications

Source: [Questions and Exercises](https://github.com/dendibakh/perf-book/blob/main/chapters/13-Optimizing-Multithreaded-Applications/13-8%20Questions-Exercises.md). The book calls this assignment `false_sharing`; its directory in this repository is `false_sharing_1`.

- [ ] [false_sharing_1](labs/memory_bound/false_sharing_1/README.md)

## Additional Practice

These assignments are part of the course but do not appear in the book's exercise lists. They are included at the end so the plan covers every assignment in this repository.

- [ ] [compiler_intrinsics_3](labs/core_bound/compiler_intrinsics_3/README.md)
- [ ] [compiler_intrinsics_4](labs/core_bound/compiler_intrinsics_4/README.md)
