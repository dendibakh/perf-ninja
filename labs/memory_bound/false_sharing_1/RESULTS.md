# False Sharing 1 — Results

## In one paragraph
Multiple threads each increment their own counter — completely independent work, no logical sharing. But all counters are packed into the same struct, and on most CPUs a cache line is 64 bytes. If two counters land in the same 64-byte cache line, every increment by thread A invalidates thread B's copy of that line (and vice versa), forcing a cache-coherence round-trip even though the threads are updating different variables.

## The mental picture
Two people are writing on the same piece of paper — one on the left half, one on the right half. They are never writing in the same spot, but every time one person writes, the other person's view of the paper is "invalidated" and they have to wait for a fresh copy before writing again. The paper is the cache line; the two people are CPU cores.

The fix: put each counter on its own *separate* cache line by padding it to 64 bytes. Now each person has their own piece of paper and never has to wait.

## Numbers
| Metric | Value |
|---|---|
| Validation | ❌ Build failed — **OpenMP not installed** on this host (`libomp-dev` missing). |
| Benchmark | Not run. |
| Expected speedup on real hardware | ≥ 60% (per lab README). |

To fix: `sudo apt-get install libomp-dev` then rebuild.

## What the optimization looks like
Add `alignas(64)` (or pad with a `char padding[64 - sizeof(T)]` member) to each per-thread counter struct so no two counters share a cache line. Each thread then has exclusive ownership of its line, and the coherence protocol never fires.

## Pattern → [PATTERNS.md: False sharing between threads](../../PATTERNS.md#false-sharing)
