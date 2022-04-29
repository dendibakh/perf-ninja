# Data packing

This is a lab about data packing.

[<img src="https://drive.google.com/uc?export=view&id=16uvUgz327TXrysAf2HXYRe_KRBALHw2j" width="30%">](https://www.youtube.com/watch?v=-V-oIXrqA2s&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

You can decrease the memory traffic of the application if you pack the data more efficiently.
Some of the ways to do that include:

* Eliminate compiler-added padding.
* Use types that require less memory or less precision e.g. (int -> short, double -> float).
* Use bitfields to pack the data even further.

[<img src="https://drive.google.com/uc?export=view&id=12iavTVH9WUbb9BguLBLKe0QqdiPBMBiG" width="30%">](https://www.youtube.com/watch?v=ta096PQ6gTg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)



```
cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
```

```
# 1. Benchmark the baseline and save score into a JSON file
./lab --benchmark_min_time=1 --benchmark_out_format=json --benchmark_out=baseline.json

# 2. Change the code

# 3. Benchmark your solution and save score into a JSON file
./lab --benchmark_min_time=1 --benchmark_out_format=json --benchmark_out=solution.json

# 4. Compare solution.json against baseline.json
python3 /home/perf-ninja/tools/benchmark/tools/compare.py benchmarks baseline.json solution.json
```

```
Comparing baseline.json to solution.json
Benchmark                         Time             CPU      Time Old      Time New       CPU Old       CPU New
--------------------------------------------------------------------------------------------------------------
bench1                         -0.1433         -0.1434           346           297           346           297
OVERALL_GEOMEAN                -0.1433         -0.1434             0             0             0             0
```
