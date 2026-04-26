# IO Optimization 1 — Results

## In one paragraph
Compute a CRC32 checksum of a file by reading it in 4-byte chunks with `read()`. The CRC computation itself is trivially cheap; the bottleneck is the cost of issuing one `read()` syscall per 4 bytes — the CPU spends most of its time in kernel mode handling syscall overhead and copying tiny buffers, rather than computing checksums.

## The mental picture
Imagine filling a swimming pool with a teaspoon. Each trip to the tap (syscall) takes the same amount of time regardless of whether you bring a teaspoon or a bucket. The time is dominated by the trips, not the water carried. Reading a file in 4-byte chunks makes millions of trips; reading in 1 MB chunks makes a handful.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **~3.5 µs** per checksum call (synthetic in-memory benchmark) |
| Stability (CV) | 1.26 % |
| Note | The benchmark simulates file IO; on a real large file the syscall overhead is proportionally much more severe. |

## What the optimization looks like
Two approaches:
1. **Large read buffers** — replace the 4-byte `read()` loop with `read(fd, buf, 1MB)` in a loop. One syscall per megabyte instead of 250 000.
2. **`mmap`** — map the file into the process address space with `mmap()`. The OS hands you a pointer; page faults bring in data on demand, and the kernel handles bulk prefetching automatically. Zero explicit `read()` calls.

## Pattern → [PATTERNS.md: IO / syscall overhead](../../PATTERNS.md#io-overhead)
