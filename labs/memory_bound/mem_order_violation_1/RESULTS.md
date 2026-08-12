# Memory Order Violation 1 — Results

## In one paragraph
Build a histogram of a grayscale image: count how many pixels have each of the 256 possible values. The loop is simple: `hist[pixel]++`. When the same colour appears in consecutive pixels (common in real images), the load for pixel N+1 arrives before the store from pixel N is committed — the CPU speculatively loads the old value and must discard it and re-execute when it detects the hazard. This is a *memory order violation* (also called a load-store forwarding conflict or store-to-load forwarding stall).

## The mental picture
Two workers share a scoreboard. Worker 1 writes "score = 5" and worker 2 immediately tries to read the same slot. If worker 2 reads before worker 1's write is visible, worker 2 gets the stale value and has to redo its work. The CPU is both workers simultaneously; when the same array slot is read right after being written, this "redo" happens frequently.

The classic fix: use multiple private histograms (one per "lane" of the loop), then merge them at the end. Now workers never share a slot, and there is no conflict.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **9.4 µs** per image histogram |
| Stability (CV) | 0.95 % |

## What the optimization looks like
Maintain 2–4 independent histogram arrays interleaved by pixel index (e.g. even pixels → hist0, odd → hist1). After the loop, add them together. The independent arrays remove the load-after-store dependency, and the merge cost is tiny (256 additions × N arrays).

## Pattern → [PATTERNS.md: Memory order violations](../../PATTERNS.md#memory-order-violation)
