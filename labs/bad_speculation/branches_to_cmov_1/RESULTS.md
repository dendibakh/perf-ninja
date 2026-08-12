# Branches to CMOVs — Results

## In one paragraph
Game of Life: simulate 10 rounds on each of 10 random 1024×1024 grids. The next state of every cell depends on how many of its 8 neighbours are alive — a simple count-and-compare. Because the grids are random, roughly half of all cells flip each round, so the "is this cell alive?" branch is correct only ~50% of the time. That is as bad as it gets for a branch predictor.

## The mental picture
Imagine you are flipping a coin and the CPU is betting heads or tails before it lands. When the coin is fair (50/50), the CPU loses the bet half the time. Every wrong bet costs ~15–20 clock cycles to undo and restart the pipeline — that wasted time shows up as **954 ms** for just ten boards.

The fix is to stop *betting* at all. A `cmov` (conditional move) instruction calculates both possible outcomes and picks the right one after the fact, without ever speculating. No bet, no penalty.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **954 ms** per call (10 boards × 10 rounds × 1 M cells) |
| Per-cell cost | ≈ 9.5 ns/cell |
| Stability (CV) | 0.28 % |

## What the optimization looks like
Replace the `if/else` in the hot inner loop with `__builtin_unpredictable(cond)` to hint to clang-17 that it should emit a `cmov` instead of a branch. No algorithm change needed — just tell the compiler the condition is unpredictable and let it choose a branchless form.

## Pattern → [PATTERNS.md: Unpredictable branches](../../PATTERNS.md#unpredictable-branches)
