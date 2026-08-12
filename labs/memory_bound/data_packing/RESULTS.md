# Data Packing — Results

## In one paragraph
Sort a large array of structs and then query them. The structs contain a mix of field types (booleans, chars, ints, doubles). The C++ compiler inserts *padding bytes* between fields to satisfy alignment requirements — a `bool` followed by an `int` will have 3 wasted bytes in between. The benchmark loads many structs from memory; each wasted padding byte is a wasted cache-line slot.

## The mental picture
Imagine packing suitcases for a trip where each suitcase holds exactly 8 items. If you pack [shirt, empty, empty, empty, book, empty, empty, empty] due to "spacing rules," you need 2 suitcases for 2 items. Re-pack as [shirt, book, ...] and you fit them in one. Fewer suitcases = fewer trips = faster.

Cache lines are the "suitcases." When structs are padded, each cache line carries fewer useful items. The CPU must fetch more cache lines per useful unit of work — this is pure memory bandwidth waste.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **41 ms** |
| Stability (CV) | 1.90 % |

## What the optimization looks like
Reorder struct fields from largest to smallest type (double → int → short → char → bool), eliminating compiler-added padding. Optionally use `#pragma pack` or bitfields for further compression — at the cost of potentially slower individual field access.

## Pattern → [PATTERNS.md: Memory traffic from struct padding](../../PATTERNS.md#memory-layout)
