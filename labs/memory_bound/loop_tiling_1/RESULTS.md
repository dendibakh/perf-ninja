# Loop Tiling 1 — Results

## In one paragraph
Rotate a large 2-D matrix 90°. For an N×N matrix, a naive rotation reads row-wise (good) but writes column-wise (bad) — or vice versa depending on the direction. On a large matrix, the column-wise writes evict every cache line before it is reused, turning the operation into pure cache-miss-dominated work.

## The mental picture
Imagine moving furniture from one room to another: room A has furniture arranged left-to-right, room B needs it top-to-bottom. Moving one piece at a time means you are constantly walking far across room B for each placement.

Tiling (blocking) says: instead of moving one piece at a time across the entire matrix, work in small square *tiles* that fit in cache. Move all pieces within one tile before moving to the next. The tile fits in L1/L2 cache, so all the reads and writes within it are fast.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **9.5 ms** |
| Stability (CV) | 0.77 % |

## What the optimization looks like
Add an outer loop over tiles (e.g. 64×64 blocks) and move the existing `i,j` loops inside, restricted to the current tile. The tile size is tuned so that two tiles (src + dst) fit in L2 cache simultaneously — eliminating the cold misses on the destination.

## Pattern → [PATTERNS.md: Cache tiling / blocking for re-use](../../PATTERNS.md#cache-tiling)
