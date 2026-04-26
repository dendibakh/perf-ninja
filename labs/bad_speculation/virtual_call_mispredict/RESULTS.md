# Virtual Call Mispredict — Results

## In one paragraph
An array holds objects of three different classes (mixed in random order) that all inherit a shared base class. Each iteration calls a `virtual` method on every object. A virtual call goes through a vtable pointer, so the CPU must predict *which function address* will be loaded — the *branch target*, not just whether a branch is taken. With three possible targets appearing in random order, the predictor guesses wrong often.

## The mental picture
Imagine a factory line where three different machines each need different instructions from the operator. The operator has to look at each machine's tag, find the right manual, and start reading before the machine finishes its current task. If the machines arrive in random order the operator is always flipping to the wrong page.

The fix: **sort the array by type** so machines of the same kind arrive together. Now the operator reads the same manual for a long run, and the CPU predictor locks onto one target address and is almost always right.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **405 µs** per iteration |
| Stability (CV) | 0.82 % |

## What the optimization looks like
Sort (or re-order during construction) the object array by dynamic type before the hot loop. No algorithm change — just bring objects of the same type together so the branch-target predictor sees a consistent target for a long stretch.

## Pattern → [PATTERNS.md: Branch-target misprediction](../../PATTERNS.md#branch-target-misprediction)
