# Software memory prefetching

[<img src="../../../img/SWMemPrefetch1-Intro.png">](https://www.youtube.com/watch?v=yTkaLNuUCXw&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

When the CPU data prefetcher cannot figure out the memory access pattern, software prefetching comes in handy. The idea is to use special instructions that tell the CPU: "Hey, I plan to use this memory location a bit later, could you fetch it for me while I do other stuff so it waits for me when I am back".

In GCC and CLANG, you can use `__builtin_prefetch` to ask the CPU to prefetch data. Say, for example, that you are going to access an element of array `my_array[index]`, where `index` is some random number. To prefetch it, you will use `__builtin_prefetch(&my_array[index]);` or `__builtin_prefetch(&my_array + index);`.

Prefetching can benefit the performance, but it can also hurt the performance. It benefits it if the piece of data you are trying to access is not in the data cache. It hurts it if it is. So most of the time, it pays off when there are random memory accesses on a large data structure, such as a tree or a hash map.

An additional prerequisite for the speedup with prefetching is that between the time you request prefetching, and the time you actually access your data, some time needs to pass (known as "prefetching window"). Immediately accessing data that you want to prefetch will not give the expected results.

[<img src="../../../img/SWMemPrefetch1-Summary.png">](https://www.youtube.com/watch?v=XkzTTh-CEUc&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Authored-by: @ibogosavljevic

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Due to the randomness of the inputs, the hardware prefetcher cannot help us by loading data we will need in future
loop iterations. We will need to do this manually using software prefetching via compiler intrinsics.

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

There are a few steps to consider when adapting a loop to prefetch data:

1. Ensure the data that are being prefetched are loaded sufficiently early; there is a prefetching window which must be
   satisfied between the load and the use of the data. This window is the time between the moment the memory address is known
   and the time at which the data at that address are required. If this interval is too short, the Out-Of-Order CPU engine
   cannot perform the prefetching.
2. The data should be prefetched at the absolute latest possible moment. This is because such prefetching can cause
   data relevant for intermediate loop iterations to be evicted from the cache, which in turn can add overheads due to
   subsequent cache misses.
3. Avoid loading data that are likely already in the cache.

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Try experimenting with a sensible "step" size in each iteration such that you issue a prefetching request for some value
that will be used N iterations from now. Try to tune N such that it fits the criteria laid out above.

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

In this lab, we have a large hash map of size ~32MB. Its size is such that it will not all fit into cache. This means we
will inevitably have several cache misses that will require lookups in DRAM.

If we first inspect the lab using `toplev.py` from `pmu-tools` (for example, if you have `pmu-tools` on your machine,
run `YOUR/PATH/TO/toplev.py --core S0-C0 --run-sample -l2 -v --no-desc taskset -c 0 ./lab` from the `build` directory
of this lab):

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           96.2 ms         96.1 ms            7
...
C0    FE               Frontend_Bound                      % Slots                       21.1    [ 8.0%]
C0    BAD              Bad_Speculation                     % Slots                        8.5  < [ 8.0%]
C0    BE               Backend_Bound                       % Slots                       52.9    [ 8.0%]
C0    RET              Retiring                            % Slots                       17.4  < [ 8.0%]
C0    FE               Frontend_Bound.Fetch_Latency        % Slots                       17.8    [ 8.0%]
C0    FE               Frontend_Bound.Fetch_Bandwidth      % Slots                        3.8  < [ 8.0%]
C0    BAD              Bad_Speculation.Branch_Mispredicts  % Slots                        8.4  < [ 8.0%]
C0    BAD              Bad_Speculation.Machine_Clears      % Slots                        0.2  < [ 8.0%]
C0    BE/Mem           Backend_Bound.Memory_Bound          % Slots                       37.7    [ 8.0%]<==
C0    BE/Core          Backend_Bound.Core_Bound            % Slots                       15.2    [ 8.0%]
C0    RET              Retiring.Light_Operations           % Slots                        9.2  < [ 8.0%]
C0    RET              Retiring.Heavy_Operations           % Slots                        8.2  < [ 8.0%]
C0-T0 MUX                                                  %                              8.00  
C0-T1 MUX                                                  %                              8.00  
```

It becomes clear that we are memory bound; most of the time is spent retrieving data from memory. This is quite easy to
see by inspecting the code.

If we look at the loop in `solution.cpp`:

```c++
  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
```

`lookups` is a vector containing over one million randomly generated integers. If we look at the implementation of the
`find` function of the `hash_map` class:

```c++
bool find(int val) const {
  int bucket = val % N_Buckets;
  return m_vector[bucket] != UNUSED;
}
```

We see that we are essentially performing random lookups of memory addresses in a huge ~32MB vector. This is a recipe
for expensive cache misses and huge performance penalties.

### Solution

Due to the randomness of the inputs, the hardware prefetcher cannot help us by loading data we will need in future loop
iterations. We will need to do this manually using software prefetching.

There are a few steps to consider when adapting a loop to prefetch data:

1. Ensure the data that are being prefetched are loaded sufficiently early; there is a prefetching window which must be
   satisfied between the load and the use of the data. This window is the time between the moment the memory address is known
   and the time at which the data at that address are required. If this interval is too short, the Out-Of-Order CPU engine
   cannot perform the prefetching.
2. The data should be prefetched at the absolute latest possible moment. This is because such prefetching can cause
   data relevant for intermediate loop iterations to be evicted from the cache, which in turn can add overheads due to
   subsequent cache misses.
3. Avoid loading data that are likely already in the cache.

Changes should always be benchmarked. Software prefetching is often difficult to get right, and when done wrong, can
have a noticeable deleterious impact on the performance of our code.

We can introduce a "step" into our loop; by this, on the `i`th iteration, we signal to the CPU to prefetch the value we
will need on the `i + step`th iteration. Much experimentation can be performed here to find the best step, but a
sensible starting point would be the width of a cache line. In this way, we minimise the risk of our prefetches
unnecessarily overlapping with data we already have in the cache.

As our vector contains `int`s, which we assume are 4 bytes, then we can set our step to be 16.

To achieve software prefetching, we need to use the compiler builtin `__builtin_prefetch`. Let's add a `prefetch_find`
function to the map:

```c++
void prefetch_find(int val) const {
    int bucket = val % N_Buckets;
    __builtin_prefetch(&m_vector[bucket]);
}
```

Then, let's rewrite our loop in `solution.cpp`:

```c++
static constexpr auto prefetch_step = 16;
int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

  for (auto i = 0; i + prefetch_step < lookups.size(); i++) {
    if (const int val = lookups[i]; hash_map->find(val))
      result += getSumOfDigits(val);
    hash_map->prefetch_find(lookups[i + prefetch_step]);
  }

  for (auto i = lookups.size() - prefetch_step; i < lookups.size(); i++) {
    if (const int val = lookups[i]; hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
```

First, we have defined our `prefetch_step` and set it to be 16 (feel free to experiment with other values to see if you
can get better performance!). We then modify our loop to trigger prefetches of `lookups[i + prefetch_step]`. We then
iterate through the final <16 elements from `i = lookups.size() - prefetch_step` onwards.

Running this code and analysing with `pmu-tools` shows:

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           43.5 ms         43.5 ms           24
...
C0    FE               Frontend_Bound                      % Slots                       25.9    [ 8.0%]
C0    BAD              Bad_Speculation                     % Slots                       11.6  < [ 8.0%]
C0    BE               Backend_Bound                       % Slots                       41.0    [ 8.0%]
C0    RET              Retiring                            % Slots                       21.6    [ 8.0%]
C0    FE               Frontend_Bound.Fetch_Latency        % Slots                       21.4    [ 8.0%]
C0    FE               Frontend_Bound.Fetch_Bandwidth      % Slots                        4.4  < [ 8.0%]
C0    BAD              Bad_Speculation.Branch_Mispredicts  % Slots                       11.5  < [ 8.0%]
C0    BAD              Bad_Speculation.Machine_Clears      % Slots                        0.1  < [ 8.0%]
C0    BE/Mem           Backend_Bound.Memory_Bound          % Slots                       24.0    [ 8.0%]<==
C0    BE/Core          Backend_Bound.Core_Bound            % Slots                       17.0    [ 8.0%]
C0    RET              Retiring.Light_Operations           % Slots                       10.1  < [ 8.0%]
C0    RET              Retiring.Heavy_Operations           % Slots                       11.5    [ 8.0%]
C0-T0 MUX                                                  %                              8.00  
C0-T1 MUX                                                  %                              8.00  
```

Naturally, we are still memory bound, but it has decreased from 37.7% to 24.0% of resources. Secondly, we notice the
benchmark runs significantly faster: 43.5 ms for 24 iterations vs. 96.1 ms for 7 iterations. This translates to
1.81 ms/iteration vs. 13.7 ms/iteration - a speed increase of almost 87%! The penalty of cache/TLB misses is extreme and
anything that can be done to minimise it will make code noticeably faster.
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/tree/golden/labs/memory_bound/swmem_prefetch_1)

</details>