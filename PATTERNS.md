# Performance Patterns — Cross-Lab Guide

This guide organises all 25 perf-ninja labs into the **recurring patterns** that cause slowdowns. Use it as a "symptom → diagnosis → lab" lookup. When you see a particular profile signature, jump to the matching pattern and study the listed labs.

> Baseline numbers below were measured on a QEMU VM (SSE4.2 only, 8 vCPU @ 2592 MHz, Ubuntu 24.04, clang-17). Numbers on real bare-metal hardware will differ — especially for vectorization labs where AVX2/AVX-512 lanes are available.

---

## Table of Contents
1. [Unpredictable Branches](#unpredictable-branches)
2. [Branch-Target Misprediction](#branch-target-misprediction)
3. [Serial Dependency Chains](#serial-dependency-chains)
4. [Missed SIMD Opportunities](#missed-simd)
5. [AoS Layout Blocking SIMD](#aos-layout-blocking-simd)
6. [SIMD with Variable-Length Work per Lane](#simd-variable-lane-work)
7. [Function Call Overhead / Missing Inlining](#function-call-overhead)
8. [Memory Access Pattern (Column-wise / Stride)](#memory-access-pattern)
9. [Cache Tiling / Blocking for Re-use](#cache-tiling)
10. [Memory Layout (Struct Padding / Data Packing)](#memory-layout)
11. [Memory Alignment](#memory-alignment)
12. [Memory Order Violations](#memory-order-violation)
13. [TLB Pressure / Random Access over Large Data](#tlb-pressure)
14. [Unpredictable Pointer Chasing](#pointer-chasing)
15. [False Sharing Between Threads](#false-sharing)
16. [IO / Syscall Overhead](#io-overhead)
17. [Profile-Guided and Whole-Program Optimization](#pgo-lto)

---

## Unpredictable Branches

### What it looks like in a profile
- High `BR_MISP_RETIRED` counter or "Bad Speculation" in Top-Down analysis.
- High % of time in `bad_speculation` bucket.
- Hot branch instructions with ~50% taken-rate.

### The core idea
The CPU speculatively executes one branch path before knowing the outcome. When it guesses wrong, it throws away ~15–20 cycles of pipeline work and starts over. For *predictable* branches (always/never taken, or in a simple pattern) the cost is zero. For *data-dependent random branches* the cost is severe.

### The intuition
The branch predictor is a pattern-matcher. Give it a detectable pattern (TTTTTTTT, TNTNTNTNT, etc.) and it locks on instantly. Give it a coin flip and it cannot do better than 50%.

### Key fixes
- **Branchless arithmetic**: replace `if (a > b) x = a; else x = b` with `x = (a > b) ? a : b` and let the compiler emit `cmov`.
- **`__builtin_unpredictable(cond)`**: tells clang "this branch is data-dependent random — please avoid branching."
- **Lookup tables**: replace a cascade of comparisons with a single array index.
- **Conditional stores**: write unconditionally, advance pointer conditionally.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [branches_to_cmov_1](labs/bad_speculation/branches_to_cmov_1/RESULTS.md) | 954 ms | Game of Life cell survival: random 50/50 branches |
| [conditional_store_1](labs/bad_speculation/conditional_store_1/RESULTS.md) | 235 µs | Key-value filter with random keys |
| [lookup_tables_1](labs/bad_speculation/lookup_tables_1/RESULTS.md) | 4.85 ms | Range bucketing via nested if/else |

---

## Branch-Target Misprediction

### What it looks like in a profile
- High `INDIRECT_BRANCH_MISPREDICTED` counter.
- Hot vtable dispatch loops.
- Profile shows time evenly spread across many virtual-method implementations.

### The core idea
For direct branches (`if/else`, `for`) the CPU predicts *whether* to jump. For indirect branches (virtual calls, function pointers) the CPU must predict *where* to jump. With N callee classes in random order, the predictor sees a random stream of N different targets.

### The intuition
Sorting all objects of the same type together turns a random stream of targets into long runs of the same target — the predictor locks on and achieves near-zero miss rate for the interior of each run.

### Key fixes
- **Sort by type** before the hot loop.
- **Devirtualize**: use templates or `std::variant` + `std::visit` so the call target is known at compile time.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [virtual_call_mispredict](labs/bad_speculation/virtual_call_mispredict/RESULTS.md) | 405 µs | Three-class hierarchy in random order |

---

## Serial Dependency Chains

### What it looks like in a profile
- Low IPC (instructions per cycle) despite the CPU being "busy."
- High `CYCLE_ACTIVITY.STALLS_TOTAL` with low memory stalls (rules out cache miss).
- Long latency chains visible in dependency-analysis tools (LLVM-MCA, Intel IACA).

### The core idea
Instruction-level parallelism (ILP) lets the CPU execute many independent instructions simultaneously. A dependency chain forces sequential execution: each instruction waits for the result of the previous one. The *critical path length* — the longest chain — is what determines latency, not the total number of instructions.

### The intuition
Think of making a multi-course meal. Independent tasks (salad, soup, dessert prep) can run in parallel. But if dessert requires the soup's leftover stock, and the soup requires the salad's discarded vinegar, you have a 3-step chain that serialises everything.

### Key fixes
- **Break artificial dependencies**: replace a single running total with 2–4 independent partial sums and combine at the end.
- **Interleave independent chains**: instead of `A→B→C; D→E→F`, do `A; D; B; E; C; F` — the CPU overlaps A/D, B/E, C/F.
- **Per-element state**: give each element its own state (e.g. per-particle RNG) instead of sharing global state.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [dep_chains_1](labs/core_bound/dep_chains_1/RESULTS.md) | 94 ms | Linked-list lookup: serial pointer chase |
| [dep_chains_2](labs/core_bound/dep_chains_2/RESULTS.md) | 16 ms | Particle simulation: global RNG state chain |

---

## Missed SIMD Opportunities

### What it looks like in a profile
- High backend stall count with integer or FP execution units not saturated.
- Assembly shows scalar (`addss`, `mulsd`, byte-by-byte loads) where vector instructions (`addps`, `vpmultiply`, `vmovdqu`) should appear.
- `-Rpass-missed=loop-vectorize` from clang reports why the loop wasn't vectorized.

### The core idea
SIMD instructions process N values in the same time as 1 scalar instruction (N = 4 for float32 with SSE, 8 with AVX2, 16 with AVX-512). When the compiler cannot prove safety (aliasing, dependencies, non-power-of-2 types), it falls back to scalar. Manual intrinsics or code restructuring can unlock the speedup.

### The intuition
Imagine stacking 8 pancakes and flipping them all at once vs. flipping one at a time. If the griddle is big enough (wide enough SIMD registers), you always want to flip in bulk.

### Key fixes
- Use compiler intrinsics (`_mm_add_epi32`, `_mm_cmpeq_epi8`, etc.) to explicitly spell out SIMD.
- Widen integer types to avoid carries (e.g. `uint32` instead of `uint16` for accumulation).
- Add `#pragma clang loop vectorize(enable)` or `__builtin_assume_aligned`.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [compiler_intrinsics_1](labs/core_bound/compiler_intrinsics_1/RESULTS.md) | 19 µs | Sliding-window sum: scalar reduction |
| [compiler_intrinsics_2](labs/core_bound/compiler_intrinsics_2/RESULTS.md) | ~3.6 ns | Byte scan for `\n`: scalar byte loop |
| [vectorization_1](labs/core_bound/vectorization_1/RESULTS.md) | 2.7 ms | Sequence alignment: recurrence blocks auto-vectorization |
| [vectorization_2](labs/core_bound/vectorization_2/RESULTS.md) | 31 µs | Checksum carry arithmetic blocks auto-vectorization |

---

## AoS Layout Blocking SIMD

### What it looks like in a profile
- Same as Missed SIMD, plus the data is laid out as array-of-structs (AoS): `[x0,y0,z0, x1,y1,z1, …]`.
- Any attempt to vectorize requires gathering non-contiguous elements.

### The core idea
SIMD loads contiguous memory. AoS layout interleaves fields from different logical records, making it impossible to load 8 `x` values in one operation — you always load `x,y,z,x,y,z,…`. The *structure of arrays* (SoA) layout flips this: `[x0,x1,x2,…][y0,y1,y2,…][z0,z1,z2,…]`.

### The intuition
A library with books shelved as [Chapter1-Book1, Chapter1-Book2, Chapter1-Book3] (SoA) lets you grab all first chapters at once. Shelved as [Chapter1-Book1, Chapter2-Book1, Chapter3-Book1, Chapter1-Book2, …] (AoS) you have to hunt.

### Key fixes
- Convert hottest fields to SoA (`float xs[], ys[], zs[]` instead of `Point pts[]`).
- Or use SIMD gather intrinsics (`_mm_i32gather_epi32`) — typically less efficient than SoA but avoids a full layout rewrite.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [compiler_intrinsics_3](labs/core_bound/compiler_intrinsics_3/RESULTS.md) | 5.2 µs | 3-D position average: interleaved XYZ blocks gather |

---

## SIMD with Variable-Length Work per Lane

### What it looks like in a profile
- SIMD is being used but utilization is low (many masked-off lanes near end of computation).
- Per-pixel or per-element iteration counts vary widely.

### The core idea
When different SIMD lanes finish at different iterations, you must mask off completed lanes while others continue. A loop that exits when *all* lanes are done wastes cycles on already-finished lanes. The key insight: the wasted work is bounded by `max_iter - min_iter` per SIMD group, which is often small compared to `max_iter`.

### Key fixes
- Use SIMD masking: compute a "still active" bitmask each iteration; only update active lanes; exit when mask == 0.
- Alternatively sort work items by expected iteration count and process similar items together.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [compiler_intrinsics_4](labs/core_bound/compiler_intrinsics_4/RESULTS.md) | 817 ms | Mandelbrot: variable escape iteration count per pixel |

---

## Function Call Overhead / Missing Inlining

### What it looks like in a profile
- Hot function with significant prologue/epilogue instruction count relative to body.
- Caller has many `call`/`ret` instructions in a tight loop.
- `perf annotate` shows time in function preamble, not computation.

### The core idea
Every function call saves and restores registers (prologue/epilogue), sets up a new stack frame, and jumps. For a function whose body is 2 instructions, the overhead may be 10+ instructions — a 5× tax. Worse: the compiler cannot optimize across the call boundary (no constant propagation, no register sharing).

### Key fixes
- Move hot, small functions to header files (implicit inline) or mark `__attribute__((always_inline))`.
- Enable `-flto` / `-flto=thin` to allow the linker to inline across translation units.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [function_inlining_1](labs/core_bound/function_inlining_1/RESULTS.md) | 708 µs | Sort comparator in a separate TU, called millions of times |
| [lto](labs/misc/lto/RESULTS.md) | 955 ms | AOBench math helpers across TU boundaries |

---

## Memory Access Pattern (Column-wise / Stride)

### What it looks like in a profile
- High LLC (last-level cache) miss rate.
- `CYCLE_ACTIVITY.STALLS_L3_MISS` high.
- Access stride > cache-line width (64 bytes).

### The core idea
CPUs prefetch cache lines sequentially. Any access pattern that jumps more than 64 bytes at a time defeats the prefetcher. Column-wise access in a row-major matrix jumps exactly `row_width × sizeof(element)` bytes — often thousands of bytes — per step.

### Key fixes
- **Loop interchange**: swap inner/outer loop indices so the innermost index walks contiguous memory.
- **Transpose**: physically rearrange the data so both passes are row-major.
- **Tiling**: process small blocks that fit in cache before moving on (see Cache Tiling).

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [loop_interchange_1](labs/memory_bound/loop_interchange_1/RESULTS.md) | 424 ms | Matrix power: column-wise inner loop |
| [loop_interchange_2](labs/memory_bound/loop_interchange_2/RESULTS.md) | N/A (IO error) | Gaussian blur: vertical pass column access |

---

## Cache Tiling / Blocking for Re-use

### What it looks like in a profile
- High cache miss rate despite sequential-ish access.
- Two nested loops with large working sets.
- The same data is loaded from memory multiple times unnecessarily.

### The core idea
When a matrix operation reads row A and column B, by the time column B is fully consumed, row A has been evicted from cache. Processing data in small *tiles* (blocks) that fit entirely in L2/L3 cache keeps both row and column hot, eliminating the re-fetch.

### Key fixes
- Add an outer tile loop; restrict the existing `i,j` loops to `[tile_i, tile_i + TILE] × [tile_j, tile_j + TILE]`.
- Tune TILE experimentally — typical values: 32–128 for L1, 256–1024 for L2.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [loop_tiling_1](labs/memory_bound/loop_tiling_1/RESULTS.md) | 9.5 ms | Matrix rotation with large matrices |

---

## Memory Layout (Struct Padding / Data Packing)

### What it looks like in a profile
- `sizeof(MyStruct)` is larger than the sum of its fields.
- High memory bandwidth utilization despite few logically accessed elements.

### The core idea
The compiler pads structs to satisfy alignment constraints. Padding bytes are never read — they are pure waste, diluting the density of useful data in each cache line. Fitting more useful bytes per cache line means fewer cache-line fetches per element.

### Key fixes
- Reorder fields from largest to smallest type.
- Use `static_assert(sizeof(MyStruct) == expected)` to catch regressions.
- Use bitfields for boolean / small-integer fields, if access is not on the critical path.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [data_packing](labs/memory_bound/data_packing/RESULTS.md) | 41 ms | Padded struct reduces useful bytes per cache line |

---

## Memory Alignment

### What it looks like in a profile
- `MEM_INST_RETIRED.SPLIT_LOADS` counter elevated.
- `L1_BOUND → Split Loads` in Intel VTune Top-Down.
- SIMD code performs slower than expected despite good cache use.

### The core idea
A SIMD load of 16 or 32 bytes that crosses a cache-line boundary requires two cache-line fetches and a hardware "split register" to merge them. The number of split registers is small; when they fill, loads stall.

### Key fixes
- Allocate SIMD buffers with `alignas(64)` or `std::aligned_alloc(64, size)`.
- Pad rows to a multiple of 64 bytes.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [mem_alignment_1](labs/memory_bound/mem_alignment_1/RESULTS.md) | 115 ms | Matrix rows misaligned → split SIMD loads |

---

## Memory Order Violations

### What it looks like in a profile
- `MACHINE_CLEARS.MEMORY_ORDERING` counter elevated.
- High `ld_blocks.store_forward` count.
- Simple histogram/accumulation loop is slower than expected arithmetic cost.

### The core idea
When a store and a subsequent load target the same address, the CPU tries to "forward" the store value directly to the load (store-to-load forwarding). If the CPU speculatively loads *before* checking whether a pending store targets the same address, and it turns out one does, it must flush and re-execute — a machine clear.

### Key fixes
- Use multiple independent histograms / accumulators and merge at the end.
- Unroll the inner loop so different iterations access different histogram slots.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [mem_order_violation_1](labs/memory_bound/mem_order_violation_1/RESULTS.md) | 9.4 µs | Image histogram: adjacent pixels update same bins |

---

## TLB Pressure / Random Access over Large Data

### What it looks like in a profile
- `DTLB_LOAD_MISSES.MISS_CAUSES_A_WALK` elevated.
- High TLB miss rate in VTune / perf.
- Access pattern is irregular (hash map, sparse matrix, tree traversal) over a large (> ~4 MB) dataset.

### The core idea
The TLB translates virtual to physical addresses. Its capacity is limited (~1 500 entries × 4 KB = ~6 MB coverage). When the working set is large and access is random, every few accesses cause a TLB miss and a multi-level page-table walk (hundreds of cycles). Huge pages (2 MB each) give 512× more coverage per TLB entry.

### Key fixes
- Use `madvise(ptr, size, MADV_HUGEPAGE)` or `mmap(MAP_ANONYMOUS | MAP_HUGETLB)`.
- Jemalloc and TCMalloc can use huge pages automatically for large allocations.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [huge_pages_1](labs/memory_bound/huge_pages_1/RESULTS.md) | 3.87 s | FEM gather/scatter over large random-access arrays |

---

## Unpredictable Pointer Chasing

### What it looks like in a profile
- High L2 or L3 miss rate.
- Pointer loads dominate the hot loop.
- Hardware prefetcher is helpless (irregular stride).

### The core idea
Pointer chasing (linked lists, trees, graphs) always reads an address from memory before it knows the *next* address to fetch. The hardware prefetcher detects sequential and strided patterns but cannot predict "follow this pointer." Each dereference pays the full L2/L3 miss penalty (~10–200 ns) in serial.

Software prefetching kicks off the next cache miss *before* it is needed, hiding the latency inside other useful work.

### Key fixes
- Insert `__builtin_prefetch(node->next)` a few iterations before the actual access.
- Prefetch distance should approximate cache-miss latency / loop-iteration time.
- For trees: prefetch both children when visiting a parent.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [swmem_prefetch_1](labs/memory_bound/swmem_prefetch_1/RESULTS.md) | 45 ms | Irregular pointer traversal; HW prefetcher cannot help |
| [dep_chains_1](labs/core_bound/dep_chains_1/RESULTS.md) | 94 ms | Linked list lookup (also a serial dependency chain) |

---

## False Sharing Between Threads

### What it looks like in a profile
- Multi-threaded code scales poorly (1.5× speedup on 8 cores).
- `MEM_TRANS_RETIRED.REMOTE_HIT` or `OFFCORE_REQUESTS.ALL_DATA_RD` elevated.
- Profile shows time in `lock xadd` or memory-fence-related instructions.

### The core idea
Cache coherence operates at the *cache-line granularity* (64 bytes), not individual variables. If thread A and thread B modify different variables that happen to sit in the same 64-byte line, every modification by one thread invalidates the other thread's copy — the line bounces between cores via the coherence protocol, creating a "hot line" bottleneck even with zero logical sharing.

### Key fixes
- Pad per-thread data to 64 bytes with `alignas(64)`.
- Use thread-local storage (`thread_local`) for per-thread accumulators.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [false_sharing_1](labs/memory_bound/false_sharing_1/RESULTS.md) | ❌ (needs OpenMP) | Per-thread counters packed in same cache line |

---

## IO / Syscall Overhead

### What it looks like in a profile
- High `%sys` CPU time in `perf stat`.
- Many `read()`/`write()` syscalls with tiny transfer sizes.
- Time dominates in kernel, not in the computation function.

### The core idea
Each `read()` syscall crosses the user/kernel boundary (~100–1000 ns), copies data, and returns. For tiny buffers the overhead per byte dwarfs the actual transfer. Larger buffers amortize the overhead; `mmap` eliminates explicit syscalls entirely.

### Key fixes
- Buffer IO into 64 KB–1 MB chunks.
- Use `mmap` for read-heavy workloads.
- Use `readahead()` or `posix_fadvise(FADV_SEQUENTIAL)` to hint prefetching.

### Labs
| Lab | Baseline | Bottleneck detail |
|---|---|---|
| [io_opt1](labs/misc/io_opt1/RESULTS.md) | ~3.5 µs (synthetic) | CRC32: 4-byte read() per input word |

---

## Profile-Guided and Whole-Program Optimization

### What it looks like
- Code is "fast enough" but not as fast as hand-tuned.
- Cross-module calls prevent inlining.
- Branch layout not optimized for actual workload.

### The core idea
- **LTO**: the compiler can only optimize within one translation unit at a time. LTO lets it see and optimize *across* all TU boundaries simultaneously.
- **PGO**: the compiler makes conservative assumptions about branch probabilities and inlining thresholds. PGO feeds it real execution data so it can make informed decisions.

Both are "free" performance (zero source change) if the workload is representative.

### Labs
| Lab | Baseline | Optimization |
|---|---|---|
| [lto](labs/misc/lto/RESULTS.md) | 955 ms | Add `-flto` |
| [pgo](labs/misc/pgo/RESULTS.md) | 4.41 s | Instrument → profile → recompile |

---

## Quick-Reference Matrix

| Profile symptom | Pattern | Labs |
|---|---|---|
| ~50% branch taken rate | Unpredictable branches | branches_to_cmov_1, conditional_store_1, lookup_tables_1 |
| Random vtable target stream | Branch-target mispredict | virtual_call_mispredict |
| Low IPC, no cache miss | Serial dep chain | dep_chains_1, dep_chains_2 |
| Scalar loops, should be SIMD | Missed SIMD | compiler_intrinsics_1/2, vectorization_1/2 |
| Interleaved struct fields | AoS blocking SIMD | compiler_intrinsics_3 |
| Variable work per SIMD lane | SIMD masking | compiler_intrinsics_4 |
| Hot function prologue | Missing inlining | function_inlining_1, lto |
| Column/stride access | Memory access pattern | loop_interchange_1/2 |
| Large matrix, high LLC miss | Cache tiling | loop_tiling_1 |
| sizeof(struct) > sum(fields) | Struct padding | data_packing |
| Split load counter high | Misalignment | mem_alignment_1 |
| Machine clear / store-fwd | Mem order violation | mem_order_violation_1 |
| TLB miss, large random data | TLB / huge pages | huge_pages_1 |
| Irregular pointer loads | Pointer chasing + SW prefetch | swmem_prefetch_1 |
| Multi-threaded, poor scaling | False sharing | false_sharing_1 |
| High %sys, tiny IO | Syscall overhead | io_opt1 |
| Cross-TU calls not inlined | LTO | lto |
| Branch layout wrong for data | PGO | pgo |
