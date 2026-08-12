# Conditional Store — Results

## In one paragraph
A large array of `(key, value)` pairs is scanned and only items whose key passes a random test are kept. This is the heart of many filter operations — texture compression, query engines, particle systems. The random keys mean the branch that decides "keep this item?" is correct only about half the time, causing continuous pipeline flushes.

## The mental picture
Picture a supermarket checkout where a conveyor belt brings random items and a cashier either bags them or sets them aside based on a barcode scan. If every barcode outcome is a surprise, the cashier keeps mis-reaching for a bag before the scan completes. A smarter approach: *always* place the item tentatively in the bag and only *commit* (advance the bag pointer) if the scan says yes — no pre-emptive reach, no waste.

That "optimistic write, conditional advance" is exactly the branchless conditional store pattern.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **235 µs** per filter pass |
| Stability (CV) | 1.21 % |

## What the optimization looks like
Write the value unconditionally to the output array, then increment the write pointer only if the condition is true. The store itself is harmless even when wasted; what matters is that there is no branch to mispredict. The compiler can express this as a `cmov` on the pointer increment.

## Pattern → [PATTERNS.md: Unpredictable branches](../../PATTERNS.md#unpredictable-branches)
