# Compiler Intrinsics 3 — Results

## In one paragraph
Compute the average (x, y, z) position of 16 384 3-D points, where each point is stored as three consecutive `uint32_t` values: `[x0, y0, z0, x1, y1, z1, …]`. The baseline accumulates each coordinate separately in a scalar loop. The compiler cannot auto-vectorize this because x, y, z are *interleaved* in memory — it would need to load non-contiguous elements to fill a SIMD register.

## The mental picture
Imagine sorting a long rope of coloured beads (red=x, green=y, blue=z, red=x, …) where you want the total length of each colour. A scalar approach picks beads one at a time. A SIMD approach needs to grab 4 reds at once — but they are not adjacent; they have 2 other colours in between. The trick is to explicitly *deinterleave* (gather) them: load three SIMD registers and "unzip" them so one holds all the reds, one all the greens, one all the blues, then sum each separately.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **5.2 µs** for 16 384 positions |
| Per-position cost | ≈ 0.32 ns |
| Stability (CV) | 1.36 % |

## What the optimization looks like
Load three SIMD registers at a time (12 bytes = one AoS triplet × 4 packed together) and use SIMD shuffle/unpack intrinsics to deinterleave into separate x, y, z accumulators. Then sum the accumulators horizontally at the end.

## Pattern → [PATTERNS.md: AoS layout blocking SIMD](../../PATTERNS.md#aos-layout-blocking-simd)
