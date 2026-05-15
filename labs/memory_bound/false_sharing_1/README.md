[<img src="../../../img/FalseSharing1.png">](https://www.youtube.com/watch?v=uRmQSHsZoxE&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)


This lab assignment focuses on improving performance by eliminating false sharing. In this lab, we
have several threads that modify data located close together in memory in parallel. This causes a lot
of overhead, because the individual cores must transfer cache lines containing the modified data amongst
themselves to satisfy cache coherence.

Your task here is to eliminate the false sharing by making sure that each thread will access a separate
cache line.

Expected speedup: at least 60%.

Authored-by: Jakub Beránek (@Kobzol)

## Hints

<details>
<summary><b>Hint 1:</b></summary>

How many `Accumulator` objects fit on a cache line on your system? Using what you know about CPU caches, what could be
an issue that arises due to multithreaded writes of the same cache line?

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

`Accumulator` has size 4, so 16 of them fit into a cache line on x86. There is a fairly high probability that at least
one thread will try to update an `Accumulator` that belongs to a cache line accessed by another thread. Your processor
needs to ensure that all cores that access the atomic variable wrapped in `Accumulator` (and hence the cache line it
lives on) see the most recent update. What performance impact does this have on threads that had previously read the old
cache line before an `Accumulator` in it was updated?

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

To avoid false sharing, you should ensure that each `Accumulator` object lives on its own cache line.

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

This lab runs in a multithreaded environment, where multiple threads are tasked to increment a container of atomic
integers wrapped in a struct, `Accumulator`.

The code is run multiple times, spawning a different number of threads, `thread_count`, on each run. A container
`std::vector<Accumulator>` contains `thread_count` `Accumulator` objects; for example, if three threads run, then there
will be three `Accumulators` in the vector.

A performance benchmark for this original code:

```
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
bench1/iterations:20/real_time        209 ms          209 ms           20

 Performance counter stats for './lab':

    27,735,792,018      task-clock                       #    6.619 CPUs utilized             
             1,357      context-switches                 #   48.926 /sec                      
                27      cpu-migrations                   #    0.973 /sec                      
             1,566      page-faults                      #   56.461 /sec                      
     5,618,330,777      instructions                     #    0.05  insn per cycle            
   107,505,349,386      cycles                           #    3.876 GHz                       
       526,104,194      branches                         #   18.968 M/sec                     
         1,232,123      branch-misses                    #    0.23% of all branches           

       4.190116386 seconds time elapsed
```

If we analyse using `perf report ./lab`, we find it reports the hotspot is the atomic addition step,
`target.value += item % 13`:

```
        │    // Write result to accumulator                                                                                                                                                                               ▒
        │    target.value += item % 13;                                                                                                                                                                                   ▒
     47 │      mov     eax,edx                                                                                                                                                                                            ▒
        │      imul    rax,rax,0x4ec4ec4f                                                                                                                                                                                 ▒
        │      shr     rax,0x22                                                                                                                                                                                           ▒
    199 │      lea     r8d,[rax+rax*2]                                                                                                                                                                                    ▒
     60 │      lea     eax,[rax+r8*4]                                                                                                                                                                                     ▒
        │      sub     edx,eax                                                                                                                                                                                            ▒
        │    operator--() volatile noexcept                                                                                                                                                                               ▒
        │    { return __atomic_sub_fetch(&_M_i, 1, int(memory_order_seq_cst)); }                                                                                                                                          ▒
        │                                                                                                                                                                                                                 ▒
        │    __int_type                                                                                                                                                                                                   ▒
        │    operator+=(__int_type __i) noexcept                                                                                                                                                                          ▒
        │    { return __atomic_add_fetch(&_M_i, __i, int(memory_order_seq_cst)); }                                                                                                                                        ▒
  91892 │      lock    add     DWORD PTR [rdi],edx                                                                                                                                                                        ▒
    621 │      add     rcx,0x4                                                                                                                                                                                            ▒
        │      cmp     rcx,rsi                                                                                                                                                                                            ▒
    189 │    ↑ jne     70     
```

Given we are working in a multithreaded environment, it is also sensible to observe the Cache-To-Cache (C2C) analysis
(using `perf c2c record ./lab` to get the data and `perf c2c report` to review it). We see a considerable number of
shared cache lines and the cache line distribution shows this sharing is being caused by the writes to
the atomic variables we have placed in our `std::vector<Accumulator>`:

```
=================================================
    Global Shared Cache Line Event Information   
=================================================
  Total Shared Cache Lines          :         32
  Load HITs on shared lines         :      87678
  Fill Buffer Hits on shared lines  :         81
  L1D hits on shared lines          :       4225
  L2D hits on shared lines          :         53
  LLC hits on shared lines          :      83318
  Load hits on peer cache or nodes  :          0
  Locked Access on shared lines     :      87498
  Blocked Access on shared lines    :          0
  Store HITs on shared lines        :      64939
  Store L1D hits on shared lines    :      64934
  Store No available memory level   :          0
  Total Merged records              :     147818
```

```
=================================================
      Shared Cache Line Distribution Pareto      
=================================================
#
#        ----- HITM -----  ------- Store Refs ------  --------- Data address ---------                      ---------- cycles ----------    Total       cpu                                  Shared                       >
#   Num  RmtHitm  LclHitm   L1 Hit  L1 Miss      N/A              Offset  Node  PA cnt        Code address  rmt hitm  lcl hitm      load  records       cnt                          Symbol  Object        Source:Line  No>
# .....  .......  .......  .......  .......  .......  ..................  ....  ......  ..................  ........  ........  ........  .......  ........  ..............................  ...  .................  ....
#
  ----------------------------------------------------------------------
      0        0    37147    27050        0        0      0x5555555da540
  ----------------------------------------------------------------------
           0.00%   12.80%    5.06%    0.00%    0.00%                0x10     0       1      0x555555561004         0      3102       609     6474         1  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    0.01%    0.00%    0.00%    0.00%                0x10     0       1      0x555555561007         0     38184         0        6         1  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%   10.26%   11.61%    0.00%    0.00%                0x14     0       1      0x555555561004         0      3005       294     6963         2  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%   10.26%   11.40%    0.00%    0.00%                0x18     0       1      0x555555561004         0      2798       640     6903         7  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    0.00%    0.00%    0.00%    0.00%                0x18     0       1      0x555555561007         0     60801         0        1         1  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%   11.22%   12.18%    0.00%    0.00%                0x1c     0       1      0x555555561004         0      3066       570     7485         8  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    0.00%    0.00%    0.00%    0.00%                0x1c     0       1      0x555555561007         0     34044         0        1         1  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%   11.76%   12.52%    0.00%    0.00%                0x20     0       1      0x555555561004         0      3125       831     7878         6  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    0.01%    0.00%    0.00%    0.00%                0x20     0       1      0x555555561007         0     45967         0        2         1  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%   12.80%   13.80%    0.00%    0.00%                0x24     0       1      0x555555561004         0      3126       825     8659         7  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    0.00%    0.00%    0.00%    0.00%                0x24     0       1      0x555555561007         0     47485         0        2         2  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%   11.96%   13.32%    0.00%    0.00%                0x28     0       1      0x555555561004         0      2971       239     8266         2  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    0.01%    0.01%    0.00%    0.00%                0x28     0       1      0x555555561007         0     39104         0        4         1  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    9.41%    9.99%    0.00%    0.00%                0x2c     0       1      0x555555561004         0      2903       862     6382         6  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    0.01%    0.00%    0.00%    0.00%                0x2c     0       1      0x555555561007         0     46987         0        4         3  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    6.33%    6.62%    0.00%    0.00%                0x30     0       1      0x555555561004         0      3011       712     4236         6  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    0.00%    0.00%    0.00%    0.00%                0x30     0       1      0x555555561007         0         0         0        1         1  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
           0.00%    3.15%    3.46%    0.00%    0.00%                0x34     0       1      0x555555561004         0      2850      1402     2119         8  [.] solution(std::vector<unsig  lab  atomic_base.h:418   0
```

### Solution:

Take the `Accumulator` struct:

```c++
struct Accumulator {
    std::atomic<uint32_t> value = 0;
};
```

We note `sizeof(Accumulator) == 4`. On x86 platforms, cache lines are 64 bytes wide; this means that up to 16 of these
Accumulator objects can live on the same cache line.

On x86, when an address is retrieved from memory, the whole cache line containing it is retrieved (or containing set of
cache lines for objects \>64 bytes).

When several atomic variables reside on the same cache line, if Thread A is writing to Atomic A, and if different
threads attempt to write to separate atomics (Atomic B, Atomic C, etc.) simultaneously, the system's caching protocol 
will force the other threads to reload the cache line in order to get the most recent version, which is a costly process
and degrades performance significantly.
Even though these atomic objects are unrelated, there is now a dependency between the threads with their write operations,
which is false sharing.

The resolution is to ensure that each atomic lives on its own cache line. This can be done using `alignas` on the struct
or on the atomic itself:

```c++
constexpr int CACHE_LINE_SIZE = 64;

struct alignas(CACHE_LINE_SIZE) Accumulator {
    std::atomic<uint32_t> value = 0;
};

/// OR:
struct Accumulator {
    alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> value = 0;
};
```

Running the performance analysis again after these changes shows over 80% performance increase (36.3 ms vs. 209ms):

```
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
bench1/iterations:20/real_time       36.3 ms         35.2 ms           20

 Performance counter stats for './lab':

     5,429,911,802      task-clock                       #    7.367 CPUs utilized             
             1,313      context-switches                 #  241.809 /sec                      
                51      cpu-migrations                   #    9.392 /sec                      
             1,574      page-faults                      #  289.876 /sec                      
     5,087,982,332      instructions                     #    0.26  insn per cycle            
    19,279,139,853      cycles                           #    3.551 GHz                       
       397,647,449      branches                         #   73.233 M/sec                     
           669,664      branch-misses                    #    0.17% of all branches           

       0.737028094 seconds time elapsed
```

Analysing the Cache-To-Cache report shows the cache line sharing has been eliminated completely:

```
=================================================
    Global Shared Cache Line Event Information   
=================================================
  Total Shared Cache Lines          :          0
  Load HITs on shared lines         :          0
  Fill Buffer Hits on shared lines  :          0
  L1D hits on shared lines          :          0
  L2D hits on shared lines          :          0
  LLC hits on shared lines          :          0
  Load hits on peer cache or nodes  :          0
  Locked Access on shared lines     :          0
  Blocked Access on shared lines    :          0
  Store HITs on shared lines        :          0
  Store L1D hits on shared lines    :          0
  Store No available memory level   :          0
  Total Merged records              :          0
```

_Note: a report may still show some cache line sharing, but it will be related to other processes, not the
lab code itself. This should be made clear when looking at the source of the shared cache line in the report._

</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/memory_bound/false_sharing_1/solution.cpp)

</details>