# SW Memory Prefetching 1 — Results

## In one paragraph
Traverse a data structure (tree or linked list) where the next node's address is computed from the current node's value — a random-access pattern the hardware prefetcher cannot predict. Each pointer dereference stalls the pipeline waiting for a cache miss to resolve (~100–200 ns), and the hardware prefetcher is helpless because it cannot see the pointer value before the load.

## The mental picture
You are driving to addresses using a GPS that only shows you the next turn *after* you complete the current one. You cannot look ahead, so you brake to a stop at every intersection. Software prefetching is like having a passenger read the directions a few turns ahead and radio them to you — you can start slowing down in advance and keep moving.

`__builtin_prefetch(&node->next_node)` while you are still processing the current node tells the CPU: "start fetching this address into cache now, I'll need it in a moment." If the prefetch window (time between the prefetch instruction and the actual access) is large enough, the cache miss is hidden.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **45 ms** |
| Stability (CV) | 2.17 % |
| Note | Higher CV reflects DRAM latency variance on this VM. |

## What the optimization looks like
In the traversal loop, insert `__builtin_prefetch(node->children[i])` (or the equivalent pointer N steps ahead) before you actually process `node`. The key is choosing a prefetch distance that matches the cache-miss latency: too small and the data still isn't ready; too large and it gets evicted before use.

## Pattern → [PATTERNS.md: Unpredictable memory access / pointer chasing](../../PATTERNS.md#pointer-chasing)
