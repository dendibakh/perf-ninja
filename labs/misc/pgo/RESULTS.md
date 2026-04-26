# PGO (Profile-Guided Optimization) — Results

## In one paragraph
A full LUA interpreter processes a specific set of scripts. The interpreter has many branches, virtual dispatches, and hot paths that depend entirely on what scripts it runs. Without PGO the compiler makes conservative guesses about which branches are taken most often; those guesses are often wrong, leading to suboptimal code layout, inlining decisions, and register allocation.

## The mental picture
A compiler normally optimizes for the *average* case — like a road engineer designing an intersection for all possible traffic patterns. PGO is like measuring actual traffic for a month and redesigning the intersection specifically for the real pattern: widen the road that carries 90% of the cars, add a protected turn for the lane that turns left most often.

In practice PGO does three things: (1) moves cold code out of the hot path (better instruction-cache use), (2) makes better inlining decisions (inline only actually-hot functions), and (3) guides branch prediction hints in the compiled code.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **4.41 s** per LUA script batch (no PGO) |
| Stability (CV) | 0.59 % |
| Note | PGO must be trained on the *same* script workload it will run. Using wrong training data can make things worse. |

## What the optimization looks like
Two-step build:
1. Compile with `-fprofile-generate`, run the target workload to produce `.profraw` data.
2. Recompile with `-fprofile-use=<merged.profdata>` — the compiler uses the measured hotness to guide all optimizations.

## Pattern → [PATTERNS.md: Profile-guided and whole-program optimization](../../PATTERNS.md#pgo-lto)
