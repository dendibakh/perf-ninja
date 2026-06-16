[Original C++ lab with docs and maybe video](https://github.com/dendibakh/perf-ninja/tree/main/labs/memory_bound/huge_pages_1)

Rust version is Linux only so far. The C++ original also supports macOS and Windows. Contributions welcome!

Observe the memory bottleneck:

- Build benchmark binary: `cargo bench --no-run`. It should print path to the binary.
- Confirm we're loading a lot from main memory: `perf stat -e  cache-references,LLC-loads,LLC-load-misses <binary>`. I get over 50% Last Level Cache (L3) misses, meaning those loads had to go to main memory.
- Check TLB: `perf stat -e dTLB-loads,dTLB-load-misses <binary>`. I have about 12% TLB misses (before optimization).

Optimize:

Enable huge pages on Linux (128 pages is a guess, try other numbers): `sudo bash -c 'echo 128 > /proc/sys/vm/nr_hugepages'`. If you use anonymous mmaped pages I don't think you need to mount a `hugetlbfs` filesystem [like the docs recommend](https://github.com/dendibakh/perf-ninja/blob/main/labs/memory_bound/huge_pages_1/HugePagesSetupTips.md).

I got a ~30% speedup.

