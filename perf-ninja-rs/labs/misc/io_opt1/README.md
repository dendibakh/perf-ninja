
After you optimize edit `benches/bench_io_opt1.rs` and add back the "Medium file" and "Large file" benchmarks. They should pass in reasonable time thanks to your improvements.

An included build script generates a necessary module (`src/data_paths.rs`) and some test files. If things aren't working right try `cargo clean` then `cargo build`.

`MappedFile.hpp` from the C++ original is not yet included, but you should be able to get huge speedups without it.

## Tips on finding bottleneck

The rest of this course deals with low-level CPU and memory issues. You find those with `perf` and the `pmu-tools/` repo (which I believe wraps `perf`). This lab is different. Here is a suggested approach. This is also an approach that should work for a real world program. The binary we are optimizing was built by `cargo bench` and lives in `target/release/deps/`.

**First** you have to discover whether we are spending most of the time in our code ("%usr") or waiting on a syscall ("%system"), meaning the kernel is doing something on our behalf:
```
pidstat -e ./bench_io_opt1-<hash> --bench --profile-time 5
```

This is an I/O optimization exercise, so we expect "%system" to be high. This means there's no point doing low-level CPU cycle counting or worrying about processor caches. Our program is basically idle, waiting for syscalls to return.

**Second** find out which syscalls we are blocked on. The `-D 1` says wait 1ms after startup before recording. This allows us to ignore process launch syscalls for a much cleaner output.
```
sudo perf trace -D 1 -s ./bench_io_opt1-<hash> --bench --profile-time 5
```

There will be a single syscall which is causing all our problems. Hopefully it is the one you were expecting.

**Finally** (and optionally) find out where the syscalls came from. In this case you already know, but it's worth the exercise.
```
sudo perf trace record --call-graph fp -D 1 -s ./bench_io_opt1-<hash> --bench --profile-time 5
sudo perf report
```

The `solution` and `bench1` functions were most likely inlined, so the calls to `read` will be reported from `bench_io_opt1::main`, which isn't especially useful here. Now make it faster!

