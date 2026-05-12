**TODO**: add an introductory and a summary videos.

This lab assignment focuses on improving performance by reducing the number of branch mispredictions. In this lab, we have a large collection of key-value pairs from which we select only useful items. Such algorithms are widely used in many real-world applications, for example, texture compression. Input collection contains random key values which makes it hard for a modern CPU to predict whether they should be selected or not. Your task here is to reduce the number of branch mispredictions.

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Begin by understanding why the code in the loop is hard to predict. Is there something about the nature of the input
data that can explain this?
</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

The inputs are random, so the hardware will not be able to predict if `item` will satisfy
`(lower <= item.first) && (item.first <= upper)`. Is there a way we could rewrite this loop so that it does not rely on an
`if` statement?
</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Try assuming that `item` fits the criteria and write it directly to `output[count]` at the beginning of each iteration.
What would we then need to do to `count` to ensure we only keep the `item`s that should be in `output` in following
loop iterations? Can we achieve these updates to `count` without using an `if` statement?
</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

The goal of this algorithm is to select the elements from `input` that fall within the specified range and populate
`output` with them.

Here is the original loop:
```c++
for (const auto item : input) {
    if ((lower <= item.first) && (item.first <= upper)) {
        output[count++] = item;
    }
}
```

Here is an example output obtained when running `perf stat ./lab` (`lab` being the built code), configured to run 10000
iterations:

```
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
bench1/iterations:10000        286 us          285 us        10000

 Performance counter stats for './lab':

     2,868,419,549      task-clock                       #    0.999 CPUs utilized             
                52      context-switches                 #   18.128 /sec                      
                 1      cpu-migrations                   #    0.349 /sec                      
               528      page-faults                      #  184.073 /sec                      
     6,275,596,090      instructions                     #    0.56  insn per cycle            
    11,294,443,984      cycles                           #    3.938 GHz                       
     1,808,386,567      branches                         #  630.447 M/sec                     
       425,721,046      branch-misses                    #   23.54% of all branches           

       2.870168980 seconds time elapsed
```

We see a severely high level of branch mispredictions - 23.54% is extremely high. The algorithm needs to be rewritten to
fix this.

If we look at the predicate in the loop, we see that it is unpredictable because the data passed in i.e. `item.first`
is not determinate in any way.

```c++
if ((lower <= item.first) && (item.first <= upper)) {
    output[count++] = item;
}
```

What we need to do is change the loop such that we increment `count` based on the integer representation of the
predicate, `(lower <= item.first) && (item.first <= upper)`:

```
count += lower <= item.first && item.first <= upper;
```

In the event that the predicate is true, we advance the `count` index, and we then can populate the next element in the
output array. If `count` does not get incremented, it means that the element should not be selected and is safe to be
overwritten by a future iteration.

Let `K` be the total number of elements written to `output` by this algorithm. By the time the full loop has run,
the `K`th entry of the output array will either be valid, or it will be a value that was added pre-emptively,
but whose `item.first` was ultimately not in range, so it should be ignored.

We can use `count` to tell the caller of this function the total number of valid elements in the output array. If `count`
is not incremented on the last loop iteration, `count` will equal `K-1`, which will show the `K`th element will not be
counted.

The final loop should look like this:

```c++
for (const auto item : input) {
    output[count] = item;
    count += lower <= item.first && item.first <= upper;
}
```

Running `perf stat` on the lab a second time shows the algorithm is over 4x as fast as before (65.6 microseconds vs.
286 microseconds), and the percentage of branch misses is down to `0.03%`, which is a massive improvement.

```
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
bench1/iterations:10000       65.6 us         65.6 us        10000

 Performance counter stats for './lab':

       664,023,567      task-clock                       #    0.998 CPUs utilized             
                25      context-switches                 #   37.649 /sec                      
                 1      cpu-migrations                   #    1.506 /sec                      
               528      page-faults                      #  795.153 /sec                      
     8,548,524,227      instructions                     #    3.34  insn per cycle            
     2,557,919,718      cycles                           #    3.852 GHz                       
       659,384,254      branches                         #  993.013 M/sec                     
           167,557      branch-misses                    #    0.03% of all branches           

       0.665149897 seconds time elapsed
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/bad_speculation/conditional_store_1/solution.cpp)

</details>
