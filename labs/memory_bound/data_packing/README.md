# Data packing

This is a lab about data packing.

[<img src="../../../img/DataPacking1Intro.png">](https://www.youtube.com/watch?v=-V-oIXrqA2s&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

You can decrease the memory traffic of the application if you pack the data more efficiently.
Some of the ways to do that include:

* Eliminate compiler-added padding.
* Use types that require less memory or less precision e.g. (int -> short, double -> float).
* Use bitfields to pack the data even further.

**Note 1:** Data Packing Summary video mentions branch mispredictions as a primary bottleneck for this lab. This is no longer true since the main source of branch mispredictions (std::sort) was replaced by counting sort.

**Note 2:** Bit Fields are implementation-specific. If you run the solution presented in the video on Windows, you may notice that the size of the structure is larger than you'd expect. The reason for this is a Microsoft-specific undocumented behavior that refuses to pack bit fields of varying types. Consider this if you want to pass performance tests on the Zen platform.

[<img src="../../../img/DataPacking1Summary.png">](https://www.youtube.com/watch?v=ta096PQ6gTg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Always start by declaring the largest sized objects first in a struct.

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

Can we achieve further reductions by noticing the maximum values used for the various fields of `S`?
Furthermore, for the integral fields, can bitfields be of use here?

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Some of the integral data types used here impose limits on what bitfield size can be set on them. Could we use a less
restrictive integral data type to represent `long long`, `int`, `short`, and `bool` that would allow us to optimise
them? After optimising the size, try to arrange them in a way that achieves the smallest size.

Note that now we are using bitfields, which are unaligned, simply ordering the fields from largest to smallest in the
struct will not guarantee the optimal result.

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

The struct `S` is suboptimally packed.

```c++
// FIXME: this data structure can be reduced in size
struct S {
  int i;
  long long l;
  short s;
  double d;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};
```

With the way it is above, there is extra padding in the following places:
1. 4 bytes between `int i` and `long long l`
2. 6 bytes between `short s` and `double d`
3. 7 bytes tail padding after `bool b`

This results in 17 extra bytes of padding, resulting in `sizeof(S) = 40` bytes.

### Solution
The idea is to declare the largest data members first in the class to minimise its size:

```c++
struct S {
  long long l;
  double d;
  int i;
  short s;
  bool b;

  bool operator<(const S &s) const { return this->i < s.i; }
};
```

This struct only has padding at the end, of size 1, meaning `sizeof(S) = 24` here.

It is actually possible to do better than this by using bitfields.

If we inspect the input data in the lab, we realise that the largest value that could be passed to `int i` and `short s`
is `99`. Given that integers of width 7 bits can represent numbers 0 to 127, we can make `int i` equivalent to `uint8_t`
(8 bits) rather than its usual 32 (4 bytes), and `short s` can also be reduced to 7 bits (instead of 16).

By extension, as these two numbers are multiplied to find the value of `l`, we know `l` will not exceed 10,000.
`l` can therefore be reduced to a 16-bit width. `double` is also too large a data type for the range we are considering
for division, so we can replace it with a `float`.

The boolean field can simply be one bit.

The version with bitfields:

```c++
struct S {
  long long l:16;
  float d;
  int i:7;
  short s:7;
  bool b:1;

  bool operator<(const S &s) const { return this->i < s.i; }
};
```

This eventually produces an object of `sizeof(S) = 16` bytes, with only two bytes of padding after `bool b`.

We can actually do better than this still. All the integral values can fit into an `unsigned`, so replacing `long long`
with `unsigned` allows us to reduce the size to 14-bits rather than 16 (as `2^14 - 1 = 16383 > 10000`, with 10,000 being
the largest value of `S::l` that we handle). This gives us further size reductions:

```c++
struct S {
  unsigned l:14;
  unsigned i:7;
  unsigned s:7;
  bool b:1;
  float d;

  bool operator<(const S &s) const { return this->i < s.i; }
};
```

With this arrangement, we get a final size of `sizeof(S) == 8` bytes.
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/memory_bound/data_packing/solution.h)

</details>
