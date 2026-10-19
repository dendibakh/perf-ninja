# Perf installation

```bash
git clone --depth 1 https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git

sudo apt install build-essential flex bison pkg-config libelf-dev libdw-dev libunwind-dev libaudit-dev ibslang2-dev libperl-dev python3-dev binutils-dev libtraceevent-dev

cd linux/tools/perf

make

cp perf /usr/bin

```

```bash
sudo apt install python3-venv python3-full
python3 -m venv ~/cmake-venv
~/cmake-venv/bin/pip install --upgrade cmake
~/cmake-venv/bin/cmake --version
```

### Build Lab
```bash
~/cmake-venv/bin/cmake -E make_directory build
~/cmake-venv/bin/cmake -DCMAKE_BUILD_TYPE=Release ..
~/cmake-venv/bin/cmake --build . --config Release --parallel 8
~/cmake-venv/bin/cmake --build . --target validateLab
~/cmake-venv/bin/cmake --build . --target benchmarkLab
```

### Set perf counter access
```bash
cat /proc/sys/kernel/perf_event_paranoid
sudo sysctl -w kernel.perf_event_paranoid=-1
cat /proc/sys/kernel/perf_event_paranoid
```

### Top down analisys
```bash
perf stat --topdown -a taskset -c 0 ./lab
```

### Finding hot paths
```bash
perf record -g -- taskset -c 0 ./lab
```

### Notes

After top down analisys we see that CPU stalls waiting for resources/data.

```bash
perf stat --topdown -a taskset -c 0 ./lab
2026-10-16T19:15:41+02:00
Running ./lab
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.34, 0.98, 1.06
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1     1019382454 ns   1019382516 ns            1

 Performance counter stats for 'system wide':

 %  tma_backend_bound %  tma_frontend_bound %  tma_bad_speculation      %  tma_retiring 
                52.9                  7.0                   1.0                39.1
```

