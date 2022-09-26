# Speed up data dependency chains #1.

[<img src="../../../img/DepChains1.png">](https://www.youtube.com/watch?v=nXf6MxNlXdg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Critical data dependency chains are increasingly becoming the [only thing that matters](https://easyperf.net/blog/2022/05/11/Visualizing-Performance-Critical-Dependency-Chains) for performance of a general-purpose application. That is why it is very important to identify those and know possible ways to make them run faster. On a SW level, you can sometimes occasionally introduce an artificial data dependency, which should not exist in the first place. Those cases are usually easy to find. In a contrast, some data dependency chains are inherent to a particular type of data structure.

Ahhhh, good old linked lists... Traversing a linked list is essentially a looooooooong data dependency chain. To get the node `N+1` you need to retrieve the node `N` first. Even if we set aside the problem with memory locality, a dependency chain will not go away. The data dependency effectively serializes the execution making your ILP (Instruction-Level Parallelism) be very low.

The task in this lab assignment is to look up all the values from linked list A in linked list B. This is an O(N^2) algorithm and involves a lot of pointer chasing. Both linked lists use an arena allocator to place individual nodes right next to each other, which improves memory locality. To improve performance of the benchmark in this lab assignment even further you need to overlap the execution of multiple dependency chains.

The idea for the lab was proposed by @ibogosavljevic.