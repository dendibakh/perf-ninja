# Compiler Intrinsics 1 — Results

## In one paragraph
A 1-D sliding-window sum over an array — the core of an image-smoothing (blur) kernel. For each output position, the algorithm adds one element entering the window and subtracts one leaving it, so each step is just two memory reads and two additions. It is a beautifully simple loop — but the compiler generates scalar code with one element per cycle instead of processing 4 or 8 elements at once.

## The mental picture
Suppose you are totalling a column of numbers by reading them one at a time with your finger. A calculator that can add four numbers at once (SIMD = Single Instruction, Multiple Data) would be four times faster for the same effort — but your finger is still moving one number at a time. The compiler sometimes cannot prove the memory accesses are independent enough to safely use the wide calculator; an explicit intrinsic is you telling it "I checked — go wide."

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **19 µs** per image row |
| Stability (CV) | 1.05 % |
| Hardware ceiling | SSE4.2 only on this VM (no AVX2), so max width is 128-bit / 4 × int32. |

## What the optimization looks like
Use `_mm_add_epi32` / `_mm_sub_epi32` (SSE2) intrinsics to process 4 `int32` values per instruction. The loop body shrinks from N scalar adds to N/4 SIMD adds, with the remainder handled as a scalar epilogue.

## Pattern → [PATTERNS.md: Missed SIMD — compiler needs a hint](../../PATTERNS.md#missed-simd)
