# LTO (Link Time Optimization) — Results

## In one paragraph
AOBench: a small ambient-occlusion renderer split across multiple `.cpp` files (translation units). Hot math functions are called across translation-unit boundaries. Without LTO the compiler cannot see *across* the boundary, so it cannot inline the callee into the caller, cannot specialize it for the caller's arguments, and cannot optimize register usage across the call. All those missed opportunities add up.

## The mental picture
Imagine you are a chef (the compiler) who can only see one recipe at a time. You prepare Ingredient A in one kitchen and ship it to another kitchen for final assembly. If you could see *both* recipes at once (LTO), you might realize "Ingredient A can skip the step where it is boxed and unboxed — I can pass it directly to the next step." Without seeing both, you box it anyway.

LTO gives the compiler (or linker) visibility across all translation units simultaneously, enabling inlining, dead-code elimination, and constant propagation across file boundaries.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **955 ms** per render (no LTO) |
| Stability (CV) | 0.50 % |

## What the optimization looks like
Add `-flto` (or `-flto=thin` for faster incremental builds) to both compile and link flags. No source change required. The linker reruns optimization across all object files, inlining hot cross-TU calls automatically.

## Pattern → [PATTERNS.md: Cross-translation-unit call overhead](../../PATTERNS.md#function-call-overhead)
