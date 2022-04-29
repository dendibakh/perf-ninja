This is a lab about [loop interchange](https://en.wikipedia.org/wiki/Loop_interchange).

[<img src="https://drive.google.com/uc?export=view&id=19g9RQifLdObp2mUHcaCHXwk6WCXmupZV" width="30%">](https://www.youtube.com/watch?v=TLDR_nO9XVc&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

[Matrix multiplication](https://en.wikipedia.org/wiki/Matrix_multiplication) is an important building block for many numerical algorithms. In this lab assignment, we compute the integer power of a given real square matrix.
The binary representation of the power significantly reduces the number of matrix operations. Still, the code has a major performance flaw. Your job is to find it out.

[<img src="https://drive.google.com/uc?export=view&id=1cOvE8kIF1CVAA3CGQTPaXq-1MSIe3l9q" width="30%">](https://www.youtube.com/watch?v=G6BbPB37sYg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

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
