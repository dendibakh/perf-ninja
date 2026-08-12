# Vectorization 2 — Results

## In one paragraph
Compute an Internet checksum (RFC 1071) by summing 16-bit words with carry folding. The carry arithmetic — "add the overflow bit back into the sum" — creates a dependency between each word and the previous result. Modern compilers handle simple reductions well, but the carry-bit dependency breaks auto-vectorization: the compiler cannot prove that the carry from word N doesn't affect word N+1 in a way that changes the final result.

## The mental picture
Normal addition of a long column of numbers can be split: add the left half independently, add the right half independently, combine at the end. That's what SIMD does — 8 independent partial sums, merge at the end.

The carry-bit operation breaks that split: the carry from each addition *does* affect the next. However, there is a mathematical trick: if you first sum everything as 32-bit integers (where overflow is just a large number, not a lost bit), the carries are preserved as the upper 16 bits. You can fold them back at the end in one step. This unlocks vectorization because the 32-bit additions are independent.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **31 µs** per checksum |
| Stability (CV) | 0.65 % |
| Note | No AVX2 on this VM — SSE-only. Gains from intrinsics will be smaller here than on a native machine. |

## What the optimization looks like
Widen the accumulator to `uint32_t` (or `uint64_t`), vectorize the summation, then fold the high-word carries back into the low word at the very end — one scalar reduction step after the SIMD loop.

## Pattern → [PATTERNS.md: Missed SIMD — carry/overflow dependency](../../PATTERNS.md#missed-simd)
