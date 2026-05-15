[<img src="../../../img/CompilerIntrinsics1-Intro.png">](https://www.youtube.com/watch?v=mlXw_qYRi78&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d&index=12)

This is a lab about using [compiler intrinsics](https://en.wikipedia.org/wiki/Intrinsic_function) to speed up parts of the code, where compilers fail to generate optimal code.

The kernel in this lab assignment is a part of the Average ImageSmoothing algorithm, which is reduced to 1 dimension and lacks division part. The algorithm uses sliding window approach to compute a sum in the subrange [-radius .. +radius]. It is a very fast approach compared to a classical Gaussian blur.

[<img src="../../../img/CompilerIntrinsics1-Summary.png">](https://www.youtube.com/watch?v=fP6Rhwf3rEs&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d&index=12)

Author: @adamf88.

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Can you think how using SIMD instructions here could help us process more elements at once? If you are on x86, have a look at the SSE
family intrinsics in the [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)
to see which you think would be relevant to our case. Recall the current code manipulates 8-bit integers, one at a time.

The code uses a sliding window to calculate a blur for position `pos` across a range
`[input.data() + pos - radius, input.data() + pos + radius]`, removing `input.data() + pos + radius - 1` from the
sliding window to ensure the width remains no greater than `2 * radius`. Try the `_mm_loadu_si64` SSE instruction to load
64 bits at once (i.e. 8 `pos` starting points).

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

The scalar problem uses prefix sums. To do this the SIMD way, you'll also need to know the algorithm for vectorised
prefix sums. For a vector `[1, 2, 3, 4]`, you would have:

1. Start with the base: `res = [1, 2, 3, 4]`
2. Shift base to the right by 2<sup>0</sup> = 1 and add to the result: `res = [1, 2, 3, 4] + [0, 1, 2, 3] = [1, 3, 5, 7]`
3. Shift base to the right by 2<sup>1</sup> = 2 and add: `res = [1, 3, 5, 7] + [0, 0, 1, 2] = [1, 3, 6, 9]`
4. Shift base to the right by 2<sup>2</sup> = 4 and add: `res = [1, 3, 6, 9] + [0, 0, 0, 0] = [1, 3, 6, 9]`
5. Stop as there are no more adds to perform (if there were, we would shift by 2<sup>3</sup> = 8, and so on).

Be wary that these intrinsics give us _little-endian_ binary numbers, so your shift operations with intrinsics will need
to be _left_ shifts as opposed to right (as seen above in our algorithm, which uses big-endian representation).
You'll be using `_mm_slli_si128` for these left shifts.
</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

What do we do with any remaining elements that do not fit in a block of eight and thus cannot be read by our SIMD
instructions?
</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

The image smoothing algorithm in this lab uses a sliding window approach to compute the prefix sum of a range of values
in a radius about some pivot. The goal is to convert the scalar prefix sum into a SIMD prefix sum that will allow us to
perform the calculations much faster.

To calculate the prefix sum of a vector quantity, we first start with the values in the vector. Then, we shift the
vector by one element to the right, then accumulate. For the second iteration, we shift by two, then in the next by four
etc. This algorithm therefore has logarithmic complexity, rather than the linear complexity observed with sequential
prefix sums.

It is quite difficult for the compiler to generate SIMD code for this, so it will be on us to use compiler intrinsics to
achieve the same effect.

Let's take a benchmark of the original code:

```
------------------------------------------------------------------------------
Benchmark                                    Time             CPU   Iterations
------------------------------------------------------------------------------
bench_partial_sum/iterations:100000       25.4 us         25.3 us       100000
```

### Solution

Let's begin with an SSE (Streaming SIMD Extensions) solution. In our sliding window prefix sum, when we get to position `pos` in our output array,
we need to subtract from our total the value at the position pointed to by `const uint8_t subtractVal = input.data() + pos - radius - 1`
(the value that has dropped out of our window range).
We then have to add the new value we have just added to our sliding window, `const uint8_t addVal = input.data() + pos + radius`.

First of all, we need to convert the `currentSum` that we got from the prefix sum accumulations in the sliding window of
sizes `[1, 2 * radius + 1]`) to something we can perform SIMD operations on.

We use `_mm_set1_epi16` to create eight copies of our 16-bit `currentSum` in a 128-bit vector value. This is known as
_broadcasting_ the input.

```c++
__m128_i current = _mm_set1_epi16(currentSum); // create vector of eight copies of currentSum
```

We then process eight elements of our data per iteration. Here is how we convert the required values:

- `subtractVal` - we need to use `_mm_loadu_si64` to load the next eight `uint8_t` from our input into the first 64-bits of an 128-bit vector.
- `addVal` - we need to use the same intrinsic as for `subtractVal` (`_mm_loadu_si64`).

Once we have loaded them into the vector value, we have an issue as we have eight single-byte integers packed into only the
first 64 bits of these vectors. We need to use the `_mm_cvtepu8_epi16` intrinsic to extend these single-byte integers into
2-byte integers, so we can perform arithmetic on these vectors with the `current` vector we created above.

Here's what this code looks like so far:

```c++
const uint8_t* subtract_ptr = input.data() + pos - radius - 1;
const uint8_t* add_ptr = input.data() + pos + radius;
const uint16_t* output_ptr = output.data() + pos;

__m128i current = _mm_set1_epi16(currentSum);

int i = 0;
for (; i + 7 + pos < limit; i += 8) {
    __m128i sub_packed_8 = _mm_loadu_si64(subtract_ptr + i); // load the eight `uint8_t` integers in range [subtract_ptr, subtract_ptr + 8)
    __m128i add_packed_8 = _mm_loadu_si64(add_ptr + i); // load the eight `uint8_t` integers in range [add_ptr, add_ptr + 8)
    __m128i sub_ext_16 = _mm_cvtepu8_epi16(sub_packed_8); 
    __m128i add_ext_16 = _mm_cvtepu8_epi16(add_packed_8);
}
```

When we advance our sliding window, we will add the corresponding `addVal` to, and subtract `subtractVal` from, each of the eight elements.
This will be our diff. Therefore, the SIMD equivalent of this difference will be `_mm_sub_epi16(add_ext_16, sub_ext_16)`.

From now on, we will use `v8i` to represent `__m128i` type (`v8i` meaning "vector of 8 elements"):

```c++
using v8i = __m128i;
...
v8i diff = _mm_sub_epi16(add_ext_16, sub_ext_16);
```

We then perform the vector prefix sum algorithm. One thing of importance to note! These intrinsics give us little-endian
binary numbers, meaning the most significant byte is in the largest memory address. In our algorithm above, we spoke of
doing right bit-shifts, but that was pertaining to _big-endian_ representations. For example, `32` in big-endian 8-bit
representation is `10000000`, but `00000001` in little-endian.

Therefore, in our algorithm, we will need to perform _left_ shifts when we accumulate the prefix sum of all of the
`diff` contributions across the eight 16-bit integers we are processing:

```c++
// Copied from above
v8i diff = _mm_sub_epi16(add_ext_16, sub_ext_16);

// Calculate the prefix sum of all the `diff`s for the eight elements:
v8i delta = _mm_add_epi16(diff, _mm_slli_si128(diff, 2)); // shift `diff` by 2 bytes to the left (i.e. drop one 16-bit integer) and add to `diff`
delta = _mm_add_epi16(delta, _mm_slli_si128(delta, 4)); // shift `delta` by 4 bytes to the left (i.e. drop two 16-bit integers) and add to `delta`
delta = _mm_add_epi16(delta, _mm_slli_si128(delta, 8)); // shift `delta` by 8 bytes to the left (i.e. drop four 16-bit integers) and add to `delta`
```

Then, we merge this `delta` into our baseline `current` to get the `result` for the `i`th iteration:

```c++
v8i result = _mm_add_epi16(delta, current);
_mm_storeu_si128((v8i*)(output_ptr + i), result); // store result into memory at address *(output_ptr + i)
```

Finally, we take the final value from our eight 16-bit integers and broadcast it for the next loop
iteration:

```c++
current_sum = (uint16_t)_mm_extract_epi16(result, 7); // extract last element
current = _mm_set1_epi16(current_sum); // broadcast current_sum into 128-bit register
```

At the end of the loop, we have to increment our `pos` (for writing outputs) by the cumulative value of `i` from all the
iterations:

```c++
pos += i;
```

Putting everything together:

```c++
  // --- THE CODE ABOVE HERE IS UNCHANGED ---
  ...
  // 2. main loop.
  limit = size - radius;
  const uint8_t* subtract_ptr = input.data() + pos - radius - 1;
  const uint8_t* add_ptr = input.data() + pos + radius;
  const uint16_t* output_ptr = output.data() + pos;

  using v8i = __m128i;
  v8i current = _mm_set1_epi16(currentSum);

  int i = 0;
  for (; i + 7 + pos < limit; i += 8) {
    v8i sub_pckd_8 = _mm_loadu_si64(subtract_ptr + i);
    v8i add_pckd_8 = _mm_loadu_si64(add_ptr + i);
    v8i sub_ext_16 = _mm_cvtepu8_epi16(sub_pckd_8);
    v8i add_ext_16 = _mm_cvtepu8_epi16(add_pckd_8);

    v8i diff = _mm_sub_epi16(add_ext_16, sub_ext_16);

    v8i delta = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));
    delta = _mm_add_epi16(delta, _mm_slli_si128(delta, 4));
    delta = _mm_add_epi16(delta, _mm_slli_si128(delta, 8));

    v8i result = _mm_add_epi16(delta, current);
    _mm_storeu_si128((v8i*)(output_ptr + i), result);

    currentSum = static_cast<uint16_t>(_mm_extract_epi16(result, 7));
    current = _mm_set1_epi16(currentSum);
  }
  pos += i;
  ...
  // --- THE CODE BELOW HERE IS UNCHANGED ---
```

Using these x86 compiler intrinsics, our code is over twice as fast in benchmarks!

```
------------------------------------------------------------------------------
Benchmark                                    Time             CPU   Iterations
------------------------------------------------------------------------------
bench_partial_sum/iterations:100000       10.4 us         10.4 us       100000
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/core_bound/compiler_intrinsics_1/solution.cpp)

</details>