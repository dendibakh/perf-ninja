# Function Inlining 1 — Results

## In one paragraph
Sort an array using a custom comparator passed as a template argument to `std::sort`. The comparator lives in a separate translation unit (separate `.cpp` file). Because the linker has not seen the comparator's body when compiling the sort, it cannot inline it — every comparison in the hot inner loop pays a full function-call overhead: save registers, jump, return, restore registers.

## The mental picture
A sorting algorithm makes millions of comparisons. Each comparison calls a helper function. Even if that helper does only 2 instructions of real work, the function-call ceremony (push/pop registers, `call`/`ret`) may add another 6–10 instructions around it. That overhead can easily double or triple the total instruction count just for bookkeeping.

Moving the comparator's definition to a header (so it can be inlined) or enabling Link-Time Optimization (LTO) eliminates the call overhead and — more importantly — unlocks further optimizations like register reuse across the comparison boundary.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **708 µs** per sort |
| Stability (CV) | 0.26 % |

## What the optimization looks like
Move the comparator definition into a header file (or mark it `inline`) so the compiler can see it at the call site and inline it into the sort's inner loop. Alternatively, enable LTO so the linker can inline across translation-unit boundaries.

## Pattern → [PATTERNS.md: Function call overhead / missing inlining](../../PATTERNS.md#function-call-overhead)
