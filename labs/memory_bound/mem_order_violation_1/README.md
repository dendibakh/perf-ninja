This lab features an oldie-but-goodie Otsu's thresholding[^1] to convert a grayscale[^2] image to a binary[^3] image. One of the key parts in this algorithm is calculating a histogram of a grayscale image, i.e., calculate how many times a certain color appears in the image. Since the input image is 8-bit grayscale, there are only 256 different colors.

```cpp
std::array<uint32_t, 256> hist;
hist.fill(0);
for (int i = 0; i < image.width * image.height; ++i)
  hist[image.data[i]]++;
```

The implementation of the histogram algortihm is very simple but it has one nasty property. For each pixel on the image, you need to 1) read the current value of the corresponding color of the pixel, 2) increment it and 3) store it back.

When updates of the same color in the histogram occur at relatively high rates, the processor may not have completed updating pixel `i` prior to beginning pixel `i+1`. In such cases, a processor predicts whether the value loaded for the `i+1` update will come from memory or from the `i`'s store. If from memory, the two updates can be performed in parallel, otherwise the processor must serialize the updates.

Simple example: if you have the following pixels in the image:
```
0xFF 0xFF 0xFF 0xFF 0xFF 0xFF ...
```
Then all updates to `hist[0xFF]` will be serialized.

Think about how you can workaround this problem. Hint: you can use aditional memory.

Bonus exercise1: what would be the worst and the best cases for the original implementation and your solution?

Input images were taken from here: https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html

[^1]: https://en.wikipedia.org/wiki/Otsu%27s_method
[^2]: https://en.wikipedia.org/wiki/Grayscale
[^3]: https://en.wikipedia.org/wiki/Binary_image

## Hints

<details>
<summary><b>Hint 1:</b></summary>

The array population pattern that we need to analyse is this snippet of code:

```c++
for (int i = 0; i < image.width * image.height; ++i)
    hist[image.data[i]]++;
```

We notice that there arises a problem if we encounter a long consecutive sequence in which the elements of `image.data`
are identical. For the sake of example, let's imagine that we have a block of ten elements adjacent to each other in
the `image.data` array, and they all equal `100`. Can the CPU perform the necessary modifications to `hist` out of
order?
</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

If we have a long, consecutive sequence in which the elements of `image.data` are identical, the CPU will be forced to
process these instructions on `hist` in order, as an increment operation requires knowledge of the previous value (which
means all prior increments must be complete).

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Try breaking the longer pipeline into smaller chunks to reduce the likelihood of consecutive identical elements of
`image.data` forcing in-order execution.

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

The function under benchmark is the following:

```c++
std::array<uint32_t, 256> computeHistogram(const GrayscaleImage& image) {
  std::array<uint32_t, 256> hist;
  hist.fill(0);
  for (int i = 0; i < image.width * image.height; ++i)
    hist[image.data[i]]++;
  return hist;
}
```

To understand the potential performance issue here, we need to understand how CPU pipelines work and how the current
code could pose issues.

In modern CPUs, a simplified view of how instructions are processed in a pipeline looks like the following:

1. **Instruction Fetch** - the CPU fetches the next instruction to run
2. **Instruction Decode** - the CPU then decodes this instruction into something it understands
3. **Execute** - the instruction is run
4. **Memory Access** - memory is accessed
5. **Write Back** - the result of the instruction is finalised and committed (_retired_)

Modern CPUs support both In-Order and Out-of-Order (OOO) execution. In the former, the instructions are run strictly in
program order, whereas Out-of-Order execution allows a CPU to begin pipelining and running executions of subsequent
instructions _before the completion_ of prior ones (prior in program order). OOO execution can provide a noticeable
performance benefit by getting a head start in finishing later operations.

However, there are restrictions on OOO executions; the CPU must produce the exact same result while running in OOO as
it would have done otherwise. An instruction therefore must not proceed if its execution is constrained by dependencies
on the result of earlier instructions. This fact also means that all operations must be retired in program order,
regardless of when they were executed by the CPU.

These restrictions are known as _pipeline hazards_. The hazards relevant to this lab we are solving here are known as
_data hazards_. This is when there are read and write dependencies between instructions.

If we return to the array population pattern in the code snippet above:

```c++
for (int i = 0; i < image.width * image.height; ++i)
    hist[image.data[i]]++;
```

We notice that there arises a problem if we encounter a long, consecutive sequence in which the elements of `image.data`
are identical. For the sake of example, let's imagine that we have a block of ten elements adjacent to each other in
the `image.data` array, and they all equal `100`. Can the CPU perform the necessary modifications to `hist` out of
order?

The answer is: no, it cannot. There are multiple instructions to "increment the value in memory for `hist[100]`".
A write operation that is being done to `hist[100]` in one iteration of the loop needs to know what value of `hist[100]`
it needs to increment, and it cannot know that until _after_ the previous iteration is retired. This ends up
degenerating to approximate an in-order CPU instruction pipeline, where the instructions of subsequent iterations have
to wait for the prior ones to be retired.

With the theory laid out, we can return to the performance analysis. Here is an example `perf` output for the original
code:

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bird/0           85.5 us         85.5 us        32819
coins/1          39.1 us         39.1 us        72406
pepper/2         32.4 us         32.4 us        86228
pixabay/3        14.6 ms         14.6 ms          193
```

### Solution

We can mitigate the impact of this pipeline hazard by breaking the increments of `hist[x]` into smaller, independent
subarrays, whose intermediate values we can simply accumulate at the end. Here is what the code looks like:

```c++
std::array<uint32_t, 256> computeHistogram(const GrayscaleImage& image) {
  std::array<uint32_t, 256> hist1{};
  std::array<uint32_t, 256> hist2{};
  std::array<uint32_t, 256> hist3{};
  std::array<uint32_t, 256> hist4{};

  int i = 0;
  for (; i + 3 < image.width * image.height; i+=4) {
    hist1[image.data[i]]++;
    hist2[image.data[i+1]]++;
    hist3[image.data[i+2]]++;
    hist4[image.data[i+3]]++;
  }

  for (; i < image.width * image.height; i++) {
    hist1[image.data[i]]++;
  }

  for (int j = 0; j < hist1.size(); ++j) {
    hist1[j] += hist2[j] + hist3[j] + hist4[j];
  }
  return hist1;
}
```

We break the loop up into chunks of four, and increment the corresponding indices of the four histograms. Note that
since these are stack-based arrays, it is fine to have random accesses to the `hist` elements as the memory is so close
and thereby fast.

If there are any remaining elements that were not handled by the prior blocks of four, we simply perform a maximum of
three extra increment operations on `hist1`.

Finally, we accumulate the results.

If we analyse the assembly code produced (compiling with `-O3` optimisations), we notice something very nice in the
final loop:

Loop:
```c++
  for (int j = 0; j < hist1.size(); ++j) {
    hist1[j] += hist2[j] + hist3[j] + hist4[j];
  }
```

Assembly:
```asm
.L8:
	vmovdqu	(%rax,%rbx), %ymm0
	vpaddd	(%rcx,%rax), %ymm0, %ymm0
	vpaddd	(%r12,%rax), %ymm0, %ymm0
	vpaddd	0(%r13,%rax), %ymm0, %ymm0
	addq	$32, %rax
	vmovdqu	%ymm0, -32(%rax,%rbx)
	cmpq	$1024, %rax
	jne	.L8
```

The compiler auto-vectorised our loop! Using YMM registers (256-bit), it is able to perform 256 / 32 = 8 simultaneous
additions of the `uint32_t` objects we store in our `hist[i]` arrays, providing us a huge performance benefit via SIMD.

The benchmark shows us improvements with lower runtimes (even for a larger number of iterations):
```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bird/0           67.2 us         67.2 us        41713
coins/1          32.2 us         32.2 us        86758
pepper/2         28.3 us         28.3 us        98598
pixabay/3        12.9 ms         12.9 ms          218
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/memory_bound/mem_order_violation_1/solution.cpp)

</details>