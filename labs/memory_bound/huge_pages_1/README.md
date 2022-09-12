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
