## Memory Alignment

*Yes, it is matrix multiplication... again*

Contrary to what some people believe or may have heard somewhere, memory alignment is still required in some cases to achieve optimal performance. This lab assignment is one such case. First, a little introduction.

A typical case where data alignment is important is SIMD code, where loads and stores access large chunks of data with a single operation. In most processors, the L1 cache is designed to be able to read/write data at any alignment. Generally, even if a load/store is misaligned but does not cross the cache line boundary, it won't have any performance penalty. However, when a load or store crosses cache line boundary, such access requires two cache line reads (*split load/store*). It requires using a *split register*, which keeps the two parts and once both parts are fetched, they are combined into a single register. The number of split registers is limited. When executed sporadically, split accesses generally complete without any observable performance impact to overall execution. However, if that happens frequently, misaligned memory accesses will suffer delays.

Our simple matrix multiply generates SIMD instructions. For small-size matrices we use a regular version with loop interchange to achive cache-friendly accesses and vectorized code. For larger sizes, we rolled up a version that uses Loop Blocking. We provide all the boiler-plate code so that you only need to change a couple of lines.

When a matrix is misaligned, split loads happen very frequently which causes performance problems. In this lab you need to fix that. **Important**: It's not enough to only align the offset of a matrix, but also each row of the matrix has to be aligned. To do that you can insert dummy columns.

For AVX2 code, it is enough when each row is aligned at 32-byte boundary and for SSE and ARM Neon only 16-byte alignment is required. However, AVX-512 requires 64-byte alignment. To be on the safe side, you can align at the cacheline boundary. Keep in mind, in Apple processors (such as M1, M2 and later), L2 cache operates on 128-byte cache lines.

If you're actively using Intel's topdown methodology (TMA), then it will be reflected under `Memory_Bound -> L1_Bound -> Split Loads` category. When you improve the alignment, observe the change in the following events: `mem_inst_retired.split_loads`, and `mem_inst_retired.split_stores`.

Keep in mind, that performance penalty applies to both loads *and* stores, so all matrices should be aligned.