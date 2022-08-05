# Compute CRC32 checksum of a file

A [CRC](https://en.wikipedia.org/wiki/Cyclic_redundancy_check) (cyclic redundancy check) is often used to ensure that data transmitted over a network was not corrupted en route.
This operation involves iterating through the transmitted file and updating the result in 32-bit increments.
Since computing the CRC of a 32-bit chunk is very cheap[1], this problem is bound by the speed at which we can supply these 32-bit chunks to the CPU.

We don't recommend you trying to speed up the CRC computation using compiler intrinsics, focus on the IO instead. This lab is designed to teach you how to efficiently read data from a file.
Solution ideas include:

- Reduce the overall file reading overhead by processing large chunks of data. 
- Map the contents of a file into the address space (e.g. `mmap` on Linux). Take a look at `MappedFile.hpp`.

Authored-by: @kubagalecki

---

[1]: Hardware instructions dedicated to this purpose exist.
In the case of x86, the `crc32` instruction is part of the SSE 4.2 extension, and is accessible via a compiler intrinsic.
Its cost is comparable to a single signed integer multiplication.
