[<img src="../../../img/Vectorization1-Intro.png">](https://www.youtube.com/watch?v=osfIC5uO0G8&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d&index=12)

[Sequence alignment](https://en.wikipedia.org/wiki/Sequence_alignment) is an important algorithm in many bioinformatics applications and pipelines. The goal of the alignment is to gain insights about their biological  relation. In particular, one is interested how the sequences diverged from a common ancestor by evolutionary events like point mutations or insertions and deletions in the respective sequences.
This problem, however, has quadratic complexity and optimizing it can have a great benefit in many applications.
Since many bioinformatic problems start with the alignment of millions of short sequence pieces of length 150 to 300 symbols, we can gain great performance improvements by using SIMD vectors. In this lab you will learn how the algorithm can be improved by transforming the data layout and exposing SIMD computations.

[<img src="../../../img/Vectorization1-Summary.png">](https://www.youtube.com/watch?v=OvM6eAh8wBc&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d&index=12)


Author: @rrahn.

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Using `perf` or reading the code should show you that the loops in `compute_alignment` are the bottleneck. Begin with
understanding the access pattern of the data in the two matrices, `sequences1` and `sequences2`, and see if you notice a
pattern of operations.

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

1. Each sequence matrix has dimension `sequence_count_v x sequence_size_v`, or `16 x 200`.
2. For each of the 16 rows, we perform the following:
    - Extract `i`th row from `sequences1` and `sequences2`.
    - While fixed on the `j`th element (or column) of the row `sequences2[i]`, iterate through each column of `sequences1[i]` and perform the necessary operations.
    - Repeat the above while iterating over the `j` columns of `sequences2[i]`.

The crucial thing to note here is that we are performing the exact same operation multiple times. This indicates that
there is a potential to perform these operations in parallel. How could we restructure our matrices such that their
memory layout (i.e. matrix dimension) is easier for the compiler to vectorise?

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

We should transpose our matrices. This will give us `200 x 16` and these 16 columns will hold our score data (16-bit as the result type).
That means we should get 16 * 16-bit = 256-bit width for our matrix, and this is exactly the size of the YMM/AVX registers.
This is a big step in assisting the compiler to produce auto-vectorised assembly code.

Now that the matrix is transposed, we are no longer trying to deal with scalar score accumulators in our loop
(such as `score_column`, `horizontal_gap_column`, and `last_vertical_gap`), but rather _vectorised_ values of
width `sequence_count_v = 16`. These types should be updated to `std::array`s (or an equivalent) of size 16,
and we should wrap them with a for-loop over each element to update each entry. Written in this way, the compiler should
find it easier to generate vectorised code for us.

Also make sure to update the existing loops in order to reflect the updated process; some may no longer be required.

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

In this lab, we have two sets of matrices which represent two collections of sequences of equal length. The algorithm
represents the computations that are typically done as part of an analysis known as _sequence alignment_, which relates
to analysis of DNA and RNA.

If we begin with profiling the code, we find the following performance output from Linux `perf`:

```
----------------------------------------------------------------------------------
Benchmark                                        Time             CPU   Iterations
----------------------------------------------------------------------------------
bench_compute_alignment/iterations:2000       1252 us         1252 us         2000
```

If we analyse the code, we understand the following:

1. Each matrix has dimension `sequence_count_v x sequence_size_v`, or `16 x 200`.
2. For each of the 16 rows, we perform the following:
    - Extract `i`th row from `sequences1` and `sequences2`.
    - While fixed on the `j`th element (or column) of the row `sequences2[i]`, iterate through each column of `sequences1[i]` and perform the necessary operations.
    - Repeat the above while iterating over the `j` columns of `sequences2[i]`.

The crucial thing to note here is that we are performing the exact same operation multiple times. This indicates that
there is a potential to perform these operations in parallel; in order to achieve this, the problem will need to be
refactored to allow this.

### Solution

Let's recall the dimensions of the matrix - `16 x 200`. What if we were to _transpose_ this matrix so that it had 200
columns and 16 rows instead? That would mean, for each of the 200 iterations of the rows, we would be handling operations
on just 16 elements. We have `std::uint8_t` stored in our matrix; 16 of these in a row would amount to 128 bits.
This is the same as the size of the XMM registers used for SIMD operations on vectorised data. This is starting to look
promising!

Let us first provide a utility method to allow us to transpose the matrix:

```c++
using simd_score_t = std::array<int16_t, sequence_count_v>;
using simd_sequence_t = std::array<simd_score_t, sequence_size_v>;

simd_sequence_t transpose(const std::vector<sequence_t>& vec) {
    simd_sequence_t transposed{};

    for (size_t i = 0; i < transposed.size(); ++i) {
        for (size_t j = 0; j < vec.size(); ++j) {
            transposed[i][j] = vec[j][i];
        }
    }
    return transposed;
}
```

Here, we create a transposed matrix and the size of its elements is equivalent to the `score_t` accumulator in the
original loop, which is a 16-bit integer. This means the rows of our transposed matrix will actually be 256 bits, not 128.
This is fine, however, as we will still be able to perform vectorisation using YMM registers instead.

Now that we have transposed the matrix, we now have to widen all the accumulators to equal the new width; instead of
each iteration simply handling a scalar `score_t` of size `std::uint16_t`, we want to handle a _vector_ score accumulator
containing 16 score accumulators (one for each column of our transposed input data). Remember, we are now trying to write
the algorithm in a way that will allow the compiler to generate vectorised code for us, so this approach is exactly what
we want.

Let's start by updating the `score_t` to be an array of 16 x 16-bit integers:

```c++
/// From our transpose change above...
using simd_score_t = std::array<int16_t, sequence_count_v>;
...
using score_t = simd_score_t;
```

We'll also need to change the initialised values to use `std::array::fill`:

```c++
score_t gap_open{};
gap_open.fill(-11);
score_t gap_extension{};
gap_extension.fill(-1);
score_t match{};
match.fill(6);
score_t mismatch{};
mismatch.fill(-4);
```

The other variables need to be an array width too. Here's how we update the loop when we initialise the first column of
the matrix:

```c++
for (size_t i = 1; i < score_column.size(); ++i) {
    for (size_t k = 0; k < sequence_count_v; ++k) {
        score_column[i][k] = last_vertical_gap[k];
        horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open[k];
        last_vertical_gap[k] += gap_extension[k];
    }
}
```

We do the same at the start of the main recursion loop:

```c++
/*
* Compute the main recursion to fill the matrix.
*/
for (unsigned row2 = 1; row2 <= sequence2.size(); ++row2) {
  score_t last_diagonal_score =
      score_column[0]; // Cache last diagonal score to compute this cell.
  for (size_t k = 0; k < sequence_count_v; ++k) {
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open[k];
      horizontal_gap_column[0][k] += gap_extension[k];
  }
...
}
```

The final loop should look like this:

```c++
for (unsigned row = 1; row <= sequence1.size(); ++row) {
   // Compute next score from diagonal direction with match/mismatch.
   score_t best_cell_score = last_diagonal_score;
   for (size_t k = 0; k < sequence_count_v; ++k) {
       best_cell_score[k] += sequence1[row - 1][k] == sequence2[row2 - 1][k] ? match[k] : mismatch[k];
   }
   for (size_t k = 0; k < sequence_count_v; ++k) {
       // Determine best score from diagonal, vertical, or horizontal
       // direction.
       best_cell_score[k] = std::max(best_cell_score[k], last_vertical_gap[k]);
       best_cell_score[k] = std::max(best_cell_score[k], horizontal_gap_column[row][k]);
       // Cache next diagonal value and store optimum in score_column.
       last_diagonal_score[k] = score_column[row][k];
       score_column[row][k] = best_cell_score[k];
       // Compute the next values for vertical and horizontal gap.
       best_cell_score[k] += gap_open[k];
       last_vertical_gap[k] += gap_extension[k];
       horizontal_gap_column[row][k] += gap_extension[k];
       // Store optimum between gap open and gap extension.
       last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score[k]);
       horizontal_gap_column[row][k] =
           std::max(horizontal_gap_column[row][k], best_cell_score[k]);
   }
}
```

Note, in the first inner loop over `best_cell_score`, that it is evaluated separately from the final loop, despite the
indices being the same. Why? It's a requirement so that the compiler can emit vectorised code that will allow us to
update all 16 columns of `best_cell_score` at once in the second loop. If we merge the two loops, we can only populate
one column per iteration, and we end up no better off than the original scalar code.

Finally, we prepare the result as follows:

```c++
// Report the best score.
for (size_t k = 0; k < sequence_count_v; ++k) {
  result[k] = score_column.back()[k];
}
```

Applying all these changes gives us a performance report that looks like this:

```
----------------------------------------------------------------------------------
Benchmark                                        Time             CPU   Iterations
----------------------------------------------------------------------------------
bench_compute_alignment/iterations:2000        497 us          497 us         2000
```

We have achieved roughly 60% speed improvement through this technique.

Inspecting the assembly produced by the compiler using `perf report` shows plenty of vectorised instructions using YMM registers
(256-bit) as expected from our theory:

```
     │     for (unsigned row = 1; row <= sequence1.size(); ++row) {                                                                                                                                                    ▒
     │       nop                                                                                                                                                                                                       ▒
     │     // Compute next score from diagonal direction with match/mismatch.                                                                                                                                          ▒
     │     score_t best_cell_score = last_diagonal_score;                                                                                                                                                              ▒
 559 │800:   vmovdqa      ymm5,YMMWORD PTR [rsp+0xc0]                                                                                                                                                                  ▒
     │     for (size_t k = 0; k < sequence_count_v; ++k) {                                                                                                                                                             ▒
     │       xor          eax,eax                                                                                                                                                                                      ▒
     │     score_t best_cell_score = last_diagonal_score;                                                                                                                                                              ▒
 282 │       vmovdqa      YMMWORD PTR [rsp+0xe0],ymm5       
```

</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/core_bound/vectorization_1/solution.cpp)

</details>