# Loop Interchange 1 — Results

## In one paragraph
Compute a matrix raised to an integer power using repeated matrix multiplications. The inner loop of the multiply accesses one matrix *row-by-row* (good — sequential) but the other matrix *column-by-column* (bad — stride equals the matrix width, so every element is in a different cache line). For large matrices this causes a cache miss on every single element of the column.

## The mental picture
Reading a book page by page is fast — sequential reads that your eyes (prefetcher) can handle. Reading every 500th word in a book is slow — your finger must jump far for each word, and nothing can be preloaded.

Column-wise access is the "every 500th word" pattern. Swapping the inner two loop indices (*loop interchange*) so the innermost loop is always sequential is the "read page by page" fix.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **424 ms** per matrix power |
| Stability (CV) | 4.55 % (higher noise — large working set, VM cache fluctuations) |

The high CV (~4.5%) is typical when the working set is large and the VM's shared last-level cache is occasionally evicted by other processes. The mean is still a reliable baseline for comparison.

## What the optimization looks like
Swap the `j` and `k` loop indices in the innermost matrix-multiply kernel so that the innermost access is always to a contiguous row. This turns column-wise cache misses into sequential prefetch-friendly reads. On real hardware this alone gives 5–10× speedup.

## Pattern → [PATTERNS.md: Column-wise / stride access pattern](../../PATTERNS.md#memory-access-pattern)
