# Data packing

This is a lab about data packing.

[<img src="https://drive.google.com/uc?export=view&id=16uvUgz327TXrysAf2HXYRe_KRBALHw2j" width="30%">](https://www.youtube.com/watch?v=-V-oIXrqA2s&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

You can decrease the memory traffic of the application if you pack the data more efficiently.
Some of the ways to do that include:

* Eliminate compiler-added padding.
* Use types that require less memory or less precision e.g. (int -> short, double -> float).
* Use bitfields to pack the data even further.

[<img src="https://drive.google.com/uc?export=view&id=12iavTVH9WUbb9BguLBLKe0QqdiPBMBiG" width="30%">](https://www.youtube.com/watch?v=ta096PQ6gTg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)


# Instructions

``
$ cmake -E make_directory build/baseline
$ cmake -E make_directory build/solution
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-12 -DCMAKE_CXX_COMPILER=clang++-12 -B build/baseline .
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-12 -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_CXX_FLAGS=-DSOLUTION -B build/solution .
```

## Validate
```
$ cmake --build build/baseline --target validateLab
$ cmake --build build/solution --target validateLab
```

## Benchmark
```
$ cmake --build build/baseline --target benchmarkLab
$ cmake --build build/solution --target benchmarkLab
```
