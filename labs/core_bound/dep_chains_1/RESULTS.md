# Dependency Chains 1 — Results

## In one paragraph
Look up every value of linked-list A inside linked-list B — an O(N²) search involving pointer-chasing through both lists. Each node lookup requires dereferencing a pointer whose address only becomes known after the previous dereference completes. Both lists live in contiguous arena memory (good for cache), so the bottleneck is not cache misses — it is the *serial dependency chain* of pointer loads.

## The mental picture
Think of a scavenger hunt where each clue tells you where to find the next clue. You cannot read clue 3 until you have read clue 2, which you cannot read until you have read clue 1. Even if all the clues are in the same room (everything in cache), you are still stuck going one at a time. The CPU's out-of-order engine cannot overlap these loads because each one depends on the result of the previous.

The fix: start *multiple independent scavenger hunts simultaneously*. If you kick off search threads for 8 different starting nodes at once, the CPU can overlap 8 chains of pointer loads, making much better use of its execution units.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **94 ms** for the full N² lookup |
| Stability (CV) | 1.61 % |

## What the optimization looks like
Interleave multiple list traversals in the inner loop — start N independent searches, advance each by one step per outer-loop iteration, and collect results at the end. This exposes independent load chains to the out-of-order engine, which can then overlap them (instruction-level parallelism across chains).

## Pattern → [PATTERNS.md: Serial dependency chains](../../PATTERNS.md#serial-dependency-chains)
