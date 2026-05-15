# Speed up data dependency chains #1.

[<img src="../../../img/DepChains1.png">](https://www.youtube.com/watch?v=nXf6MxNlXdg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Critical data dependency chains are increasingly becoming the [only thing that matters](https://easyperf.net/blog/2022/05/11/Visualizing-Performance-Critical-Dependency-Chains) for performance of a general-purpose application. That is why it is very important to identify those and know possible ways to make them run faster. On a SW level, you can sometimes occasionally introduce an artificial data dependency, which should not exist in the first place. Those cases are usually easy to find. In a contrast, some data dependency chains are inherent to a particular type of data structure.

Ahhhh, good old linked lists... Traversing a linked list is essentially a looooooooong data dependency chain. To get the node `N+1` you need to retrieve the node `N` first. Even if we set aside the problem with memory locality, a dependency chain will not go away. The data dependency effectively serializes the execution making your ILP (Instruction-Level Parallelism) be very low.

The task in this lab assignment is to look up all the values from linked list A in linked list B. This is an O(N^2) algorithm and involves a lot of pointer chasing. Both linked lists use an arena allocator to place individual nodes right next to each other, which improves memory locality. To improve performance of the benchmark in this lab assignment even further you need to overlap the execution of multiple dependency chains.

The idea for the lab was proposed by @ibogosavljevic.

## Hints

<details>
<summary><b>Hint 1:</b></summary>

The `n`th node in the linked list cannot be accessed before the `n-1`th element, which in turn requires the `n-2`th
element, and so on. This dependency chain cannot be broken, but can you think how we may be able to handle it better?
</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

Note from the algorithm that we compare every node in List A with every node in List B. Could we find a more efficient
way of performing these comparisons?
</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Try grouping multiple nodes from List A and comparing each _batch_ with each node of list B. Which batch size gives you
the best performance improvement, and can you think why that may be the case in theory?
</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

Dependency chains in code can lead to bottlenecks in performance. By dependency chain, we refer to sections of code
that must be done before a new task can be completed; the new task has a _dependency_ on the prior chain. This can be
problematic as it prevents the ability to parallelise workflows.

Let's consider the problem in the lab. We have a quadratic algorithm in which we take every node in a linked list and
search for a node with the same value in another list.

The dependency chain in this problem is fairly clear; the `n`th node in the linked list cannot be accessed before the
`n-1`th element, which in turn requires the `n-2`th element, and so on.

Here is a sample performance report from the original code run:
```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           85.5 ms         85.4 ms           33
```

### Solution

As there is no way to break the dependency chains here, we have to think of a more efficient strategy. Given that we
compare every node in list A to the nodes in list B, a better approach would be to compare a _group_ of nodes from A
during the visit of every node of B. By doing this, we take a set of nodes from A and loop over its entries at every
node of B. In this way, we can reduce the impact of the dependency chains by reducing the time interval between when
we compare nodes `N` and `N+1` from list A with the linked list B.

We can create a templated version of the solution function that takes as its parameter the number of nodes from A to
process per loop iteration:

```c++
template<int N>
unsigned solution(List *l1, List *l2) {
    ...
}
```

First, we count the length of list `l1`:

```c++
List* head1 = l1;
int length1 = 0;
while (head1 != nullptr) {
    length1++;
    head1 = head1->next;
}
```

Then, we compare the chunks of size `N` to each node in `l2`, shifting prematurely to the next chunk of size `N` if we
find all `N` values in `l2`:

```c++
  unsigned retVal = 0;
  List* head2 = l2;

  // Process in chunks
  for (int i = 0; i < length1 / N; i++) {
    std::array<unsigned, N> arr{};
    for (int j = 0; j < N; j++) {
      arr[j] = l1->value;
      l1 = l1->next;
    }
    int seen = 0;
    l2 = head2;
    while (l2) {
      int v = l2->value;
      for (int j = 0; j < N; j++) {
        if (arr[j] == v) {
          retVal += getSumOfDigits(v);
          if (++seen == N) {
            break;
          }
        }
      }
      l2 = l2->next;
    }
  }
```

For completeness, we have to account for any remaining elements of `l1` that were not handled in the prior blocks of `N`:

```c++
  // process any remaining elements in l1 (<N elements)
  while (l1) {
    unsigned v = l1->value;
    l2 = head2;
    while (l2) {
      if (l2->value == v) {
        retVal += getSumOfDigits(v);
        break;
      }
      l2 = l2->next;
    }
    l1 = l1->next;
  }
```

What is a sensible size of blocks of nodes to search? That is up to you to try on your machine, but notice that the
linked lists are allocated in memory via an arena allocator, meaning the elements are adjacent to one another in
contiguous memory. The CPU loads one cache line at a time, so the load of the first node from memory will also pull a
number of adjacent nodes at the same time. The struct `List` has size 16 (a pointer of size 8, and an unsigned int of
size 8):

```c++
struct List {
  List *next;
  unsigned value;
};
```

On x86 systems, a cache line is 64 bytes wide; therefore, a logical place to start would be to work with blocks of 4
`List` pointers at a time, as these point to a region in memory equal to a cache line.

Using `N = 4`, the benchmark shows a marked improvement - twice the number of iterations in half the time!

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1           34.7 ms         34.7 ms           78
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/core_bound/dep_chains_1/solution.cpp)

</details>