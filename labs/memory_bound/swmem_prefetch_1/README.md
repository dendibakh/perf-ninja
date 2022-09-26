# Software memory prefetching

[<img src="../../../img/SWMemPrefetch1-Intro.png">](https://www.youtube.com/watch?v=yTkaLNuUCXw&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

When the CPU data prefetcher cannot figure out the memory access pattern, software prefetching comes in handy. The idea is to use special instructions that tell the CPU: "Hey, I plan to use this memory location a bit later, could you fetch it for me while I do other stuff so it waits for me when I am back".

In GCC and CLANG, you can use `__builtin_prefetch` to ask the CPU to prefetch data. Say, for example, that you are going to access an element of array `my_array[index]`, where `index` is some random number. To prefetch it, you will use `__builtin_prefetch(&my_array[index]);` or `__builtin_prefetch(&my_array + index);`.

Prefetching can benefit the performance, but it can also hurt the performance. It benefits it if the piece of data you are trying to access is not in the data cache. It hurts it if it is. So most of the time, it pays off when there are random memory accesses on a large data structure, such as a tree or a hash map.

An additional prerequisite for the speedup with prefetching is that between the time you request prefetching, and the time you actually access your data, some time needs to pass (known as "prefetching window"). Immediately accessing data that you want to prefetch will not give the expected results.

[<img src="../../../img/SWMemPrefetch1-Summary.png">](https://www.youtube.com/watch?v=XkzTTh-CEUc&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Authored-by: @ibogosavljevic
