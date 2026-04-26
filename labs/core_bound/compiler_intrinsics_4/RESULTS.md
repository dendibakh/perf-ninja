# Compiler Intrinsics 4 — Results

## In one paragraph
Render the Mandelbrot set: for each pixel, repeatedly apply z = z² + c and count iterations until |z| > 2 or a maximum is reached. Every pixel is independent — the perfect SIMD candidate — but pixels need *different numbers of iterations*, so a naive SIMD version would waste cycles spinning on already-escaped pixels.

## The mental picture
Imagine four runners doing laps on a track. You want to count laps for all four simultaneously (SIMD). But runner 1 finishes after 10 laps, runner 3 after 200 laps. If you stop the whole group when runner 1 finishes, you miss the others. If you keep all four running until the last one finishes, you waste runner 1's time.

The solution is a *masking* strategy: once a pixel "escapes," mark its lane as inactive and stop updating it, but keep the other lanes going. All lanes still execute every iteration, but masked lanes contribute nothing. The wasted work on done pixels is small compared to the gain from processing 4–8 pixels simultaneously instead of 1.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **817 ms** per frame |
| Stability (CV) | 0.78 % |
| Hardware ceiling | No AVX2 on this VM; max 4 × float32 (SSE). On AVX2 you get 8 wide. |

## What the optimization looks like
Use `_mm_cmpgt_ps` to compute an escape mask each iteration. Accumulate the iteration count only for non-escaped pixels using a masked add (`_mm_andnot_ps`). Stop the loop when all lanes have escaped (`_mm_movemask_ps` == 0xF).

## Pattern → [PATTERNS.md: SIMD with variable-length work per lane](../../PATTERNS.md#simd-variable-lane-work)
