To prefetch data in Rust use:

- [std::intrinsics::prefetch_read_data](https://doc.rust-lang.org/std/intrinsics/fn.prefetch_read_data.html)
- [std::intrinsics::prefetch_write_data](https://doc.rust-lang.org/std/intrinsics/fn.prefetch_write_data.html)

Those two and their `*_instruction` equivalents become the LLVM intrinsic [llvm.prefetch](https://llvm.org/docs/LangRef.html#llvm-prefetch-intrinsic) which I suspect must become one of the asm instructions [PREFETCHh](https://www.felixcloutier.com/x86/prefetchh), so you could also call that directly:

- [core::arch::x86_64::_mm_prefetch](https://doc.rust-lang.org/core/arch/x86_64/fn._mm_prefetch.html)

---

Under Clang 14 this is not memory bound. The bottleneck appears to be branch prediction. The Rust version matches the C++ version that way. It's possible this won't teach you what the lab intends. Add prefetching anyway, and see if the benchmark shows a performance improvement.

