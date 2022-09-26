[<img src="../../../img/LoopTiling1.png">](https://www.youtube.com/watch?v=wPcDgju8VkI&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Loop tiling (blocking) is an important technique that you can use to speed up code that is working with multi-dimensional arrays. If one of the memory access patterns on your array is column-wise, or if in the code you are accessing the same data several times in the loop, this technique can be very beneficial for the performance. It is often seen in matrix multiplication and matrix rotation operations, to speed them up.

Every time the CPU loads a new element of a matrix, it also fetches a few neighboring elements (cache line) belonging to the same row. If matrices are big and you are accessing a matrix column-wise, performance of your code may suffer from poor cache utilization. Because by the time you access the second element in the first row, it's no longer in the cache since it was replaced by the cache lines with elements from other rows of the matrix.

So, instead of going through the whole matrix at once, you can split it into small chunks, which entirely fit into a CPU cache. By processing matrix in blocks (tiles), you are reusing the elements of the matrix which are in the CPU cache and this will give your code a speed boost. Picking the right value for the TILE_SIZE is experimental and depends both on the HW architecture and the algorithm itself. Hint: you can use Roofline Performance analysis (in Intel Advisor or other tools) to determine what's limiting performance of the loop.

Authored-by: @ibogosavljevic
