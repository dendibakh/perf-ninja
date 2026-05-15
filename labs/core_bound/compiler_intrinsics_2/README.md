[<img src="../../../img/CompIntrin2.png">](https://www.youtube.com/watch?v=0WUihFxjzSE&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

This is a second lab about using [compiler intrinsics](https://en.wikipedia.org/wiki/Intrinsic_function) to speed up parts of the code, where compilers fail to generate optimal code.

The task of this lab assignment is to find the longest line in a file. There is a way to find end-of-line characters in a parallel way if you utilize compiler intrinsics.

Bonus exercise: whether solution that uses intrinsics is faster than the baseline is heavily affected by the input data. Run your solution on different input files to determine the speedup/slowdown.

The idea for this lab was proposed by Yuriy Lyfenko (@obender12).

Co-authored-by: Andrew Evstyukhin (@andrewevstyukhin)

Co-authored-by: Jakub Beránek (@Kobzol)

## Hints

<details>
<summary><b>Hint 1:</b></summary>

We want to read as many `char`s at once by vectorising our code manually. Most machines should be able to handle up to
256 bits at once. Which instruction set family from [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)
would suit this in the best way?
</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

Use the AVX instructions. Now that we have 32 * 8-bit `char`s in a single vector, how can we efficiently find how many
of these characters match the newline `\n` character? Try investigating what you can do with the `_mm256_set1_epi8`,
`_mm256_cmpeq_epi8`, and `_mm256_movemask_epi8` instructions.
</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Assuming you have worked out how to use the mask to extract the positions of `\n` in a 256-bit vector,
the count of zeroes between each set bit (`1`) in the resulting mask represents the length of a line in our input. Some
lines will be greater than 32 in length, so you'll need a way to track this across iterations. Don't forget to handle any leftover
lines that are too small to fill a 256-bit vector.
</details>

<br>

<details>

<summary><b>Worked Solution:</b></summary>

## Background:

In this lab, the goal is to find the longest line in a file; more specifically, the longest continuous string of text
that occurs up to a new line character (or EOL character).

The existing code goes through character by character and compares to `\n`. An example benchmark of the performance is
as follows:

```
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
bench1/iterations:8000        344 us          344 us         8000
```

While the algorithm runs in O(n) time, where `n` is the total length of the input, we can speed up the number of
comparisons we can perform in an iteration by using compiler intrinsics.

### Solution

We can use compiler intrinsics to read 32 `char`s in a single vector instruction, and in turn,
compare them with one instruction. This should significantly improve the performance of the algorithm. Let's look at how
that would be done.

Firstly, we set up a `pos` variable to track our position along the string. Then, we check the length of the input
contents. If the input contents is greater than or equal to 32 `char`s, then we can use our SIMD code.

```c++
int pos = 0;
int len = inputContents.size();

if (len >= 32) {
    // SIMD code goes here...
}
```

We will be working with 32 `char`s at a time, which is 32 bytes = 256 bits. Let's prepare a SIMD mask of line break
characters `'\n'`:

```c++
using v32i = __m256i;
const v32i eol = _mm256_set1_epi8('\n');
```

In our loop, we will need to track the beginning position of the current line whose length we are counting, as well as a
pointer to the first element of the 32 that we load in each iteration:

```c++
uint32_t curr_begin = 0;
auto* ptr = inputContents.data();
```

Let's take a look at what the inner loop looks like and describe it step-by-step:

```c++
for (; pos + 32 < len; pos += 32) {
  v32i v = _mm256_loadu_si256(reinterpret_cast<const v32i*>(ptr));
  v32i v_mask = _mm256_cmpeq_epi8(v, eol);
  uint32_t mask = _mm256_movemask_epi8(v_mask);
  while (mask) {
    uint32_t chars = _tzcnt_u32(mask); // C++20 version: uint32_t chars = std::countr_zero(mask);
    uint32_t curr_len = (pos - curr_begin) + chars;
    if (pos < curr_begin) {
      // Case where our mask has picked up multiple '\n' in one chunk.
      // Therefore, the `curr_len` should just be the gap between adjacent '\n', which is `chars`
      curr_len = chars;
    }

    curr_begin += curr_len + 1;
    longestLine = std::max(longestLine, curr_len);

    // Prepare bit-shift to remove current line of length `chars` plus the extra '\n' character
    chars++;
    if (chars > 31) {
      break;
    }
    mask >>= chars;
  }
  ptr += 32;
}
```

Firstly, we load the next 32 characters from the address pointed to by `ptr`. Then, we perform a SIMD vector comparison
which will compare _all_ 32 characters to `'\n'` in a _single_ vectorised instruction using YMM registers, which is very
efficient. After that, we need to convert the vector output to a standard scalar `uint32_t`, whose set bits will correspond
to the characters in that set of 32 which were equal to `'\n'`:

```c++
v32i v = _mm256_loadu_si256(reinterpret_cast<const v32i*>(ptr)); // Get next 32 characters
v32i v_mask = _mm256_cmpeq_epi8(v, eol); // SIMD compare 32 characters with '\n'
uint32_t mask = _mm256_movemask_epi8(v_mask); // Convert to scalar value. Set bits correspond to positions where comparisons were equal
```

If `mask` is non-zero, then we've found a section that contains at least one `'\n'`. We count the _trailing_ zeroes in `mask`
to find the contribution to the current line length, and update `longestLine` if we have found a new longest string.
We do this for all set bits in `mask` until `mask` equals zero.

```c++
  while (mask) {
    uint32_t chars = _tzcnt_u32(mask); // C++20 version: uint32_t chars = std::countr_zero(mask);
    uint32_t curr_len = (pos - curr_begin) + chars;
    if (pos < curr_begin) {
      // Case where our mask has picked up multiple '\n' in one chunk.
      // Therefore, the `curr_len` should just be the gap between adjacent '\n', which is `chars`
      curr_len = chars;
    }

    curr_begin += curr_len + 1;
    longestLine = std::max(longestLine, curr_len);

    // Prepare bit-shift to remove current line of length `chars` plus the extra '\n' character
    chars++;
    if (chars > 31) {
      break;
    }
    mask >>= chars;
  }
  ptr += 32;
```

We then adapt the original loop that existed in the code to handle any remainder of the input string whose length is
less than 32:

```c++
  for (; pos < len; pos++) {
    curLineLength = (inputContents[pos] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }
```

The performance impact of these changes is significant. We see a run time of `86.3 us` compared to `344 us` in the original:

```
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
bench1/iterations:8000       86.3 us         86.1 us         8000
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/core_bound/compiler_intrinsics_2/solution.cpp)

</details>