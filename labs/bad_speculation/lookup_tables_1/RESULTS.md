# Lookup Tables — Results

## In one paragraph
A stream of integers in `[0, 150]` is mapped into a small set of buckets using a cascade of `if/else` comparisons. Each comparison is a branch, and because the input is random the branches are hard to predict. With many levels of comparison the misprediction cost accumulates into **4.85 ms** per batch.

## The mental picture
Imagine sorting coloured balls by reading a number on each ball and then deciding which bucket it belongs to by asking "Is it < 10? No. Is it < 50? Yes → bucket 2." Each question is a coin flip. A lookup table short-circuits all the questions: just use the number directly as an array index. One memory load, no questions, no wrong guesses.

The tradeoff: a lookup table costs memory and can cause cache pressure if it is large. For a range of 0–150 that fits in 151 bytes — trivially in cache.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **4.85 ms** per batch |
| Stability (CV) | 0.33 % |

Note: this is notably slower than the other bad-speculation labs because the baseline has *many* comparison levels, so the number of mispredictions per element is higher.

## What the optimization looks like
Build a 151-element `uint8_t bucket[151]` array at startup, then replace the entire `if/else` chain with `bucket[value]`. One load replaces N branches.

## Pattern → [PATTERNS.md: Unpredictable branches](../../PATTERNS.md#unpredictable-branches)
