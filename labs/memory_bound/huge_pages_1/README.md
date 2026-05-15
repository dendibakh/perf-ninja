# Finite element operator evaluation

## Motivation and problem description
The finite element method is a discretization technique used mainly for physics simulations.
The physical domain (usually 2D or 3D) where the physical problem is defined is broken up into a mesh of small, geometrically simple subdomains called elements (much like a surface is broken down into triangles in computer graphics).
The continuous fields representing physical quantities (e.g. density, pressure, temperature) are approximated using a finite number of values, which represent the values of the fields at the vertices (nodes) of the elements.
For this reason, these values are usually referred to as nodal values.
The partial differential equations describing the physics to be simulated are expressed as systems of linear algebraic equations, with the nodal values being the unknowns.
These algebraic systems are usually quite large, and therefore must be solved using (super-)computers.

Since the resulting system of algebraic equations is usually solved using iterative methods (e.g. the [conjugate gradient method](https://en.wikipedia.org/wiki/Conjugate_gradient_method)), it is not necessary to explicitly store the matrix describing it in memory.
It is sufficient to be able to evaluate the action of this matrix on a vector (compute the matrix-vector product).
This approach is usually referred to as matrix-free.

In this lab, you are meant to optimize a piece of code which evaluates the aforementioned matrix-vector product for a structural problem involving a truss.
The truss consists of many bars, each of which is represented using a single finite element.
To evaluate the matrix-free operator, we iterate over all elements, compute the local 4x4 matrix, gather the local 4-element left-hand side vector, perform the multiplication, and then scatter the result into the global right-hand side vector.
This example is rather simple (at the level of a 2nd year mechanical engineering course), but it illustrates the gather-scatter memory access pattern, which is ubiquitous in various simulation codes across the globe.

**Note**: The above description of the finite element method is extremely simplified, and is only meant to give a high level overview of the problem.

## Hint
Note that the compiler generates fairly optimal code for the floating-point computations, so there are little gains to be had by optimizing those.
Instead, focus on the memory access pattern, which is extremely random, meaning many distant memory addresses are accessed in rapid succession, which puts stress on the TLB.
This situation could be alleviated by allocating the memory which is accessed in a random fashion on huge pages (see [HugePagesSetupTips](HugePagesSetupTips.md)).
For the convenience of your solution, all such allocations are done using the `allocateDoublesArray` function.
In fact, this is the only place of the code which is modified in the suggested solution.

Authored-by: @kubagalecki

## Hints

<details>
<summary><b>Hint 1:</b></summary>

Using what you know about how an operating system maps between virtual memory and physical memory, what can you say about
the number of regular pages (whose size is roughly ~4kB on Linux, for example) that would be needed to map the memory in
the arrays created in this lab? Why may this number be bad for performance?

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

Recall that the Translation Lookaside Buffer (TLB), which caches memory addresses for extremely fast lookups (i.e. a handful
of CPU cycles), has a limited size. How can we make use of huge pages in our program to reduce the pressure on the TLB?

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

Set up huge pages support on your system and work out how to create the `double` arrays in `allocateDoublesArray` in
a huge page of memory, rather than a regular one. Based on the theory, what performance improvements do you expect to
see, and why?

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

This lab involves processing several arrays of doubles whose contents are in the tens of millions.
The sheer size of the arrays, as well as the random access of the elements, pose inherent issues
as these are not characteristics of a cache-friendly algorithm.

### Solution (Linux)

The problem can be alleviated slightly through the use of huge tables. Before explaining what these are
and how they work, I'll add a brief explanation of paging and virtual memory.

When a process (such as the program of our algorithm) is run by the operating system (OS),
the OS provides a virtualisation layer between the process and the machine's physical memory. This virtualisation
provides _virtual addresses_ to the process that it can use for reads and writes; unbeknownst to the process, these virtual addresses are mapped
by the OS to real physical memory addresses.

Many OS platforms, such as Linux, implement the mapping between virtual and physical memory through the use
of pages. Pages can be thought of as tables whose entries describe the mapping from virtual to physical memory. When a
process performs a lookup for a memory address for the first time, the CPU goes to main memory (RAM) to retrieve it. Since
this is an extremely slow lookup, the results of these lookups are stored in a cache on the hardware chip. Each chip has a
cache associated with it on its memory-management unit (MMU), known as the Translation Lookaside Buffer (TLB).
The idea of the TLB is to provide extremely fast lookups of data that are regularly retrieved by the process.

When mapping a large memory space, such as on the order of hundreds of megabytes or even gigabytes, the sheer number of pages
required to map the virtual memory to physical memory puts pressure on the TLB. The standard page size in Linux is 4 kB,
meaning thousands (or even millions) of pages would be needed to map all the memory used by the arrays in an algorithm like this.
This exceeds the number of entries that can be held in the TLB at a given time (which is usually around 4,000).
Combined with the observation that this algorithm performs data lookups in a random order,
a very high number of expensive TLB cache misses is likely to occur.

By using huge pages (2MB), we can drastically increase the amount of memory mapped by one page (and thus one TLB entry)
by 512 times. This reduces the risk of TLB misses, as the cache will be able to store mappings for a much larger region
of memory (as each page entry represents a region 512x larger than when using 4kB pages). This should boost performance.

When running the original code, we find the following performance:

```
-------------------------------------------------------------------------------------
Benchmark                           Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------
Apply matrix-free operator       7.09 s          7.09 s             1 bytes_per_second=206.508Mi/s
```

With the updated code that uses huge pages (2MB):

```
-------------------------------------------------------------------------------------
Benchmark                           Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------
Apply matrix-free operator       6.02 s          6.02 s             1 bytes_per_second=243.305Mi/s
```

We observe roughly 20% higher throughput of bytes and an overall reduction in runtime of around 1 second.

Below is the Linux code. It uses `mmap` and `munmap` to allocate and deallocate the memory. The `allocateDoublesArray`
function will allocate the array, and it will be backed by huge pages of size 2MB each. A `std::unique_ptr` is used with
a custom deleter, so that the allocated memory will be cleaned up automatically as soon as the object goes out of scope
in the program.

```c++
#include <sys/mman.h>
inline auto allocateDoublesArray(size_t size) {
  size_t total_array_size = size * sizeof(double);
  constexpr size_t page_2mb = 1UL << 21;
  auto pages_needed = total_array_size / page_2mb + (total_array_size % page_2mb != 0);
  size_t total_to_alloc = pages_needed * page_2mb;

  void* ptr = mmap(nullptr, total_to_alloc, PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0);
  if (ptr == MAP_FAILED) {
    throw std::bad_alloc{};
  }
  madvise(ptr, total_to_alloc, MADV_HUGEPAGE);

  auto deleter = [total_to_alloc](double *p) { munmap(p, total_to_alloc); };

  return std::unique_ptr<double[], decltype(deleter)>(static_cast<double*>(ptr), std::move(deleter));
}
```
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/memory_bound/huge_pages_1/AllocateDoublesArray.hpp)

</details>