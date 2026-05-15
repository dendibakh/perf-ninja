[<img src="../../../img/LookupTables1.png">](https://www.youtube.com/watch?v=bhz4t5QYApE&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Welcome to the next lab assignment, where we will fight branch mispredictions by replacing them with lookup tables. The code in this lab assignment maps values from `[0;150]` into buckets, which involves a lot of comparisons, and so, branches. To solve this assignment you need to figure out a way how to replace *all* hard-to-predict branches.

Bonus question: how would you solve it if the range of possible values will be bigger, say `[0;10000]`?

## Hints

<details>
<summary><b>Hint 1:</b></summary>

As always, begin by identifying the problematic section of code. `perf` is a great tool for this. You can also use
`perf stat` to confirm if the code is indeed suffering from a high rate of branch prediction errors (~10% is extremely
high).

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

While the branch taken by the `if` statements itself may not be predictable, have you noticed something about the
range of possible values `v` is being checked against? Is there anything predictable there?

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

We know the ranges of `v` that should produce a given output. Use a hash table/array that maps these values of `v` to the
expected output. To handle values of `v` that are out of range, you can use `std::min(v, largest - 1)`. Note that the
compiler should produce branchless code for `std::min` when compiled with the correct optimisations.

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

The code takes an input value, `v`, and this value is converted into an index for a bucket in an array.

The function below can almost be thought of as a very simple hashing function.

```c++
static std::size_t mapToBucket(std::size_t v) {
                              //   size of a bucket
  if      (v < 13)  return 0; //   13
  else if (v < 29)  return 1; //   16
  else if (v < 41)  return 2; //   12
  else if (v < 53)  return 3; //   12
  else if (v < 71)  return 4; //   18
  else if (v < 83)  return 5; //   12
  else if (v < 100) return 6; //   17
  return DEFAULT_BUCKET;
}
```

The performance of this code is impaired by the unpredictable nature of the branches. Analysis using `perf` gives the
following (after running `perf stat ./lab` on the build output), showing 11.93% of all branches were branch misses:

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           5475 us         5475 us          123

 Performance counter stats for './lab':

       755,117,145      task-clock                       #    0.998 CPUs utilized             
                 5      context-switches                 #    6.621 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
             2,321      page-faults                      #    3.074 K/sec                     
     3,082,840,080      instructions                     #    1.03  insn per cycle            
     2,984,387,486      cycles                           #    3.952 GHz                       
       804,485,800      branches                         #    1.065 G/sec                     
        95,962,434      branch-misses                    #   11.93% of all branches           

       0.756944478 seconds time elapsed
```

### Solution

The way to resolve this is to use a lookup table that uses the value of v to index directly into the lookup table and
return the desired result.

```c++
constexpr int TABLE_SIZE = 101;
uint8_t lookup_table[TABLE_SIZE] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  DEFAULT_BUCKET
};
static std::size_t mapToBucket(std::size_t v) {
  return lookup_table[std::min(v, TABLE_SIZE - 1)];
}
```

Now, we have greatly reduced the number of branches and the code should perform much faster.

`std::min` is often optimised by compilers to be branchless (as observed in this scenario, where the code is compiled
with `-O3` optimisations).

The branchless version achieves superior performance, as shown below (2.67 insn per cycle vs. 1.03 isns per cycle), with
only 0.01% of all branches mispredicted (vs. 11.93% above):

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1            995 us          995 us          689

 Performance counter stats for './lab':

       821,914,095      task-clock                       #    0.999 CPUs utilized             
                12      context-switches                 #   14.600 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
             2,319      page-faults                      #    2.821 K/sec                     
     7,693,561,980      instructions                     #    2.67  insn per cycle            
     2,878,375,328      cycles                           #    3.502 GHz                       
       855,928,464      branches                         #    1.041 G/sec                     
            84,302      branch-misses                    #    0.01% of all branches           

       0.823010242 seconds time elapsed
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/bad_speculation/lookup_tables_1/solution.cpp)

</details>
