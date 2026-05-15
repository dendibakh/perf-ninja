**TODO**: add an introductory and a summary videos.

This is the good old game of life. The program takes 10 randomly generated 1024x1024 grids (boards) and simulates the next 10 game of life rounds. It first simulates 10 rounds for the first board, then goes off to the second board, and so on.

As written, the program experiences many branch mispredictions. Your job is to find where they happen and replace them with predicate instructions. On x86 you should see `cmov` instructions (hint: use `__builtin_unpredictable`, make sure you have Clang-17 or later version installed). On ARM you should see `csel` (conditional select) instructions. It is a good idea to experiment with the code in godbolt.org before you start modifying the actual code.

Bonus question: when you fix the main source of mispredictions, there are still many branches left in the hot loop nest. How can you get rid of them?

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Have you found the part of the code where there may be a lot of branch mispredictions? Are there any sections where you
would find it difficult to say where the code is likely to flow based on random inputs?
</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

A good place to start would be to identify statements involving `if` or `switch`, whose outcomes cannot be easily
predicted due to the nature of the input data. These places are likely to cause problems for the hardware branch
predictor. You can confirm your thoughts by profiling with something like `perf record` and viewing the code it
highlights as the cause of branch mispredictions.
</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

The main code section to focus on is here:

```c++
// Implementing the Rules of Life:
switch(aliveNeighbours) {
    // 1. Cell is lonely and dies
    case 0:
    case 1:
        future[i][j] = 0;
        break;                   
    // 2. Remains the same
    case 2:
        future[i][j] = current[i][j];
        break;
    // 3. A new cell is born
    case 3:
        future[i][j] = 1;
        break;
    // 4. Cell dies due to over population
    default:
        future[i][j] = 0;
}
```

Notice that many of the branches can be removed by realising that `future[i][j]` will always be `0` except if
`aliveNeighbours` is `2` or `3`. After that, we can use `__builtin_unpredictable()` (if using x86 and Clang 17+) to
give a hint to the compiler that these branches will be tricky to predict. In turn, this built-in can help the compiler
produce more appropriate assembly code to reduce the impact of branching i.e. by generating `cmov` instructions.
</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

The aim of this lab is to replace unpredictable branches with conditional moves.
A conditional move `cmov` is not necessarily always faster than code with branches, but in this case, where the branches
are not predictable, we should expect (and do observe) a performance increase.

### Solution:

The main section of branches exists here:

```c++
// Implementing the Rules of Life:
switch(aliveNeighbours) {
    // 1. Cell is lonely and dies
    case 0:
    case 1:
        future[i][j] = 0;
        break;                   
    // 2. Remains the same
    case 2:
        future[i][j] = current[i][j];
        break;
    // 3. A new cell is born
    case 3:
        future[i][j] = 1;
        break;
    // 4. Cell dies due to over population
    default:
        future[i][j] = 0;
}
```

The assembly code output for the above can be viewed [here](https://godbolt.org/z/Gdza4dzze).

_(Note: The above is a code snippet compiled without optimisations for illustrative purposes; no test data are provided for `future` and `current`, so `-O3` optimises most of the assembly instructions out as it notices the value of `future[i][j]` will remain 0, i.e., unchanged, unless `aliveNeighbours == 3`)._

From this, we can note three distinct final outcomes for the value of `future[i][j]`:

1. `future[i][j] = 0` - if `aliveNeighbours` is any value other than 2 or 3.
2. `future[i][j] = current[i][j]` - remains the same if `aliveNeighbours` is equal to 2.
3. `future[i][j] = 1` - if `aliveNeighbours` equals 3.

This shows that we can begin with assuming `future[i][j]` will equal 0, and write code that changes the outcome only for
`aliveNeighbours == 2` or `aliveNeighbours == 3`. This will simplify the branches.

```c++
// Implementing the Rules of Life:
int cell_value = 0;
if (aliveNeighbours == 2) {
    cell_value = current[i][j];
} else if (aliveNeighbours == 3) {
    cell_value = 1;
}
future[i][j] = cell_value;
```

This is an improvement, but we still have not eliminated the branches entirely.

To do this, we need some help from compiler intrinsics. Given the fact that `aliveNeighbours` is not predictable, the performance of the code as it is will suffer from major branch misprediction penalties.
On `x86`, we use the built-in `__builtin_unpredictable()` on Clang (versions 17+) to indicate that this branch cannot be predicted by hardware prediction methods. This will help the compiler produce better assembly code.

```c++
// Implementing the Rules of Life:
int cell_value = 0;
if (__builtin_unpredictable(aliveNeighbours == 2)) {
    cell_value = current[i][j];
} else if (__builtin_unpredictable(aliveNeighbours == 3)) {
    cell_value = 1;
}
future[i][j] = cell_value;
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/bad_speculation/branches_to_cmov_1/solution.hpp)

</details>
