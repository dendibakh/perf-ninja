# Compiler Intrinsics 2 — Results

## In one paragraph
Find the longest line in a file by scanning for newline (`\n`) characters. The baseline scans byte-by-byte. The benchmark processes a synthetic in-memory buffer, so the bottleneck is pure CPU throughput — how quickly can we scan bytes — not disk I/O.

## The mental picture
Imagine checking a long printed list for a specific symbol by reading it letter-by-letter versus using a highlighter that can scan 16 characters at a glance. The SIMD approach loads 16 bytes at once into a register and asks "is any of these a newline?" in a single instruction — roughly 16× the throughput of the byte-by-byte scan.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **~3.6 ns** per call (very tight inner loop, tiny buffer per iteration) |
| Stability (CV) | 1.61 % |
| Note | The benchmark input is short; the ratio scales to a large speedup on real file sizes. |

## What the optimization looks like
Use `_mm_cmpeq_epi8` (SSE2) to compare 16 bytes against `\n` simultaneously, then `_mm_movemask_epi8` to extract a bitmask of matches. Process the file in 16-byte strides; handle the tail scalar. This is the same pattern used in `memchr`, `strlen`, and most fast string-scanning routines.

## Pattern → [PATTERNS.md: Missed SIMD — compiler needs a hint](../../PATTERNS.md#missed-simd)
