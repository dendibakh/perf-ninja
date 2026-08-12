# Huge Pages 1 — Results

## In one paragraph
A matrix-free finite-element operator repeatedly gathers values from randomly scattered memory locations (the "scatter/gather" access pattern common in physics simulations). The floating-point work is minimal; the bottleneck is the **Translation Lookaside Buffer (TLB)** — the hardware cache that translates virtual addresses to physical ones. With 4 KB pages the TLB covers a tiny fraction of the working set, causing a TLB miss (and a page-table walk) on a large fraction of accesses.

## The mental picture
A TLB is like a pocket address book with room for ~1 500 entries. Each entry maps one street to a house number. With 4 KB pages and gigabytes of data, your address book fills up almost instantly. Every time you need a new address you have to look it up in the full city directory (page-table walk) — expensive.

Huge pages (2 MB instead of 4 KB) mean each address-book entry covers 512× more territory. The same pocket book now covers 512× more addresses without any misses. For code with random access over large data, switching to huge pages dramatically reduces TLB pressure.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **3.87 s** per operator evaluation |
| Stability (CV) | 0.64 % |
| Note | This VM (QEMU) does not have huge pages configured, so the baseline is the worst case. The optimization requires kernel support (`madvise(MADV_HUGEPAGE)` or explicit `mmap` with `MAP_HUGETLB`). |

## What the optimization looks like
Replace the large random-access array allocations with huge-page-backed allocations via `madvise(ptr, size, MADV_HUGEPAGE)` or a custom allocator using `mmap(MAP_ANONYMOUS | MAP_HUGETLB)`. The FP computation is unchanged; only the memory backing changes.

## Pattern → [PATTERNS.md: TLB pressure / random access over large data](../../PATTERNS.md#tlb-pressure)
