# Memory Alignment 1 — Results

## In one paragraph
A matrix multiply kernel generates SIMD loads and stores. When matrix rows are not aligned to a 16-byte (SSE) or 32-byte (AVX2) boundary, a SIMD load that straddles a cache-line boundary requires *two* cache-line fetches instead of one — a "split load." In a tight loop that executes millions of SIMD loads, frequent split loads consume extra split-load hardware registers and create head-of-line blocking in the load pipeline.

## The mental picture
Think of cache lines as pages in a book, and a SIMD load as reading two words at once with two fingers. If the two words are on the same page, one page-turn is enough. If they straddle page N and page N+1, you need to open two pages simultaneously. The CPU has a limited number of "hands" for holding two pages at once (split-load buffers). When they fill up, new loads must wait.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **115 ms** |
| Stability (CV) | 0.24 % |
| Note | No AVX2 on this VM (SSE4.2 only), so alignment to 16 bytes matters most here. On AVX2 you would also align to 32 bytes. |

## What the optimization looks like
Allocate each matrix with `alignas(64)` (or `std::aligned_alloc`) and pad each row to a multiple of the cache-line width. This guarantees that every SIMD load falls within a single cache line, eliminating all split loads.

## Pattern → [PATTERNS.md: Misaligned SIMD memory access](../../PATTERNS.md#memory-alignment)
