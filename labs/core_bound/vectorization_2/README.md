[<img src="../../../img/Vectorization2_button.png">](https://www.youtube.com/watch?v=m4SWal8EAgM&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

This is a second lab about [auto vectorization](https://llvm.org/docs/Vectorizers.html). The subject of this lab assignment is a part of a checksum algorithm from the 80s, which has risen from the popularity of the Internet and [accompanying needs to validate transmitted packets](https://www.alpharithms.com/internet-checksum-calculation-steps-044921/). Even the problem is old, similar issues may exist nowadays in production code.

Modern compilers handle simple loops very well, including horizontal additions. In this lab computations inside the loop are slightly more difficult: we do an "add carry" operation. Some compilers recognize "add carry" and others don't. The [carry flag](https://en.wikipedia.org/wiki/Binary_number#Binary_arithmetic) is still a dark area in C++ while it exists more than 40 years. In this lab assignment, you will practice fixing auto-vectorization, which will improve performance significantly.

Hint: the [RFC 1071](http://www.faqs.org/rfcs/rfc1071.html) paper in the section "2. Calculating the Checksum" describes possible techniques to speed up this assignment. Also, clang can help to find [causes](https://llvm.org/docs/Vectorizers.html#diagnostics) of bad performance.

## Hints

<details>
<summary><b>Hint 1:</b></summary>

The add carry operation (due to the potential overflow) makes it hard for the compiler to vectorise this code. A normal
approach to attempting to solve this issue is to widen the accumulators we use in our loop. Here, we are using
`uint16_t` for `acc`. How could this be updated to help us begin to break the dependency between the carry and the add?

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

Try using `uint32_t` for `acc`. This will essentially allow us to accumulate two lots of 16-bit integers (or two of the
"old" `acc`s at once).

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Overflows should be handled carefully. Try limiting the number of elements from `blob` that you process in one go (i.e. batch of integers), such
that you will never observe more than one overflow in `acc` in this batch. Without this, it is possible for an overflow to happen in
both the low 16-bits _and_ in the high, and it is possible to miss these by not staggering our consumption of `blob` and periodically
checking for overflows.

Don't forget to combine the top and bottom 16-bit integers for the final result (as well as any overflows caused by
that operation!).

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

This lab is based on a basic algorithm that computes the checksum of a set of data. The use of checksums is widespread;
a common example would be in networking. A checksum is generated from some data in the payload, and it is used by
recipients of said data as one level of verification of the data in the packet.

The code is very simple:

```c++
uint16_t checksum(const Blob &blob) {
  uint16_t acc = 0;
  for (auto value : blob) {
    acc += value;
    acc += acc < value; // add carry
  }
  return acc;
}
```

Looking at the assembly (compiled with `-O3`):

```asm
checksum(std::array<unsigned short, 1024ul> const&):
        lea     rdx, [rdi+2048]
        xor     eax, eax
.L8:
        add     ax, WORD PTR [rdi]
        adc     ax, 0
        add     rdi, 2
        cmp     rdi, rdx
        jne     .L8
        ret
```

We see that the compiler has generated two instructions for the two additions in the loop; a regular add of `value` to
the `ax` register, and then an `adc` instruction, which is an add carry. Essentially, this instruction will add 1 to the
value of `acc` if the line prior (`acc += value`) resulted in an unsigned integer overflow in `acc`.

Here is a benchmark run for the original code:

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           33.4 us         33.3 us        83412
```

This code cannot be vectorised easily because of the dependency that the second potential addition has on the outcome
of the first operation. We need to find a way to decouple these.

### Solution

As in most problems involving vectorisation, we should try to find a method to widen the size of an accumulator to allow
more computations to be done. In this case, we could use a _32-bit_ unsigned integer rather than 16-bit one. This would
help us effectively accumulate _two_ `acc`; one in the lower 16 bits and another in the high 16. We then track overflows
of the 32-bit integer at the end of processing each chunk of data, and add our carry value once.

Let's begin to modify the code. First, we should set `acc` to be `uint32_t`:

```c++
uint32_t acc = 0;
```

Next, we want to add as many integers from `blob` as possible in chunks so that we get no more than one 32-bit overflow
per chunk of additions. This is quite easy to achieve; the largest value we will encounter from blob is 2<sup>16</sup> - 1, so
if we never process more than 2<sup>15</sup> (`1 << 16`) numbers at a time, we will not cause the 32-bit accumulator
to overflow more than once.

We therefore introduce a chunk to the loop:

```c++
constexpr size_t chunk_size = (1 << 16);
for (size_t i = 0; i < N; i += chunk_size) {
    for (size_t j = i; j < i + chunk_size && j < N; j++) {
        ...
    }
}
```

Now, rather than checking for add carry operations every loop iteration, we do it after every chunk is processed. To do
this, after every iteration, we need to remember what the value of `acc` was in the previous iteration,
and compare it to new value of `acc`. If `acc` is smaller than the value it had at the end of the previous loop, then
we know we have seen an unsigned integer overflow, and we increment `acc` by `1`.

```c++
constexpr size_t two_pow_16 = (1 << 16);
for (size_t i = 0; i < N; i += two_pow_16) {
    uint32_t acc = 0, prev = 0;
    for (size_t j = i; j < i + two_pow_16 && j < N; j++) {
        acc += blob[j];
    }
    if (acc < prev) {
        acc++;
    }
    prev = acc;
}
```

Once we have completed our accumulation, it's time to combine the top and bottom 16-bits of `acc` to get our final answer.

```c++
uint16_t top = acc >> 16;
uint16_t bottom = acc & (two_pow_16 - 1);
uint16_t ans = top + bottom;
ans += ans < top;
return ans;
```

Note that the sum itself can overflow, so we add in an extra 1 to our answer if it does. Note that it does not matter
which of `top` or `bottom` we choose to compare `ans` to.

Here is the final code:

```c++
uint16_t checksum(const Blob& blob) {
  constexpr std::size_t two_pow_16 = 1 << 16;
  uint32_t acc = 0, prev = 0;
  for (std::size_t i = 0; i < N; i += two_pow_16) {
    for (std::size_t j = i; j < i + two_pow_16 && j < N; j++) {
      acc += blob[j];
    }
    if (acc < prev) {
      acc++;
    }
    prev = acc;
  }
  uint16_t top = acc >> 16;
  uint16_t bottom = acc & (two_pow_16 - 1);
  uint16_t ans = top + bottom;
  ans += ans < top;
  return ans;
}
```

The performance report shows it is almost ten times as fast this way!

```
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
bench1/iterations:83000       3.61 us         3.61 us        83000
```
</details>