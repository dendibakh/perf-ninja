# Loop Interchange 2 — Results

## In one paragraph
Apply a Gaussian blur to a large grayscale image using two passes of a 1-D filter (horizontal then vertical). The vertical pass reads pixel columns — one pixel per row, skipping `width` bytes between each read. On a large image this causes frequent cache misses, exactly like the matrix column-access problem in Loop Interchange 1.

## The mental picture
The horizontal pass scans across a row — sequential, cache-friendly, fast. The vertical pass scans down a column — each step jumps one full image width in memory, so every pixel load is likely a cache miss on a large image.

One fix (loop interchange) re-orders the loops so the innermost access is always horizontal. Another fix pre-transposes the image in memory so the second pass also reads rows. Both turn the cache-unfriendly column walk into a cache-friendly row scan.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ (build ok) |
| Benchmark | ❌ **"An IO problem"** — the benchmark requires an input image file that was not present in the build directory on this host. |
| Expected baseline | Similar order to loop_interchange_1 (hundreds of ms for a large image). |

To reproduce: provide an input image (e.g. a large PGM/PPM file) at the path the benchmark expects, then re-run `./lab`.

## What the optimization looks like
Interchange the row/column loops in the vertical pass so the innermost index walks across columns (contiguous in memory) rather than down rows. Alternatively, transpose the intermediate buffer so both passes are horizontal.

## Pattern → [PATTERNS.md: Column-wise / stride access pattern](../../PATTERNS.md#memory-access-pattern)
