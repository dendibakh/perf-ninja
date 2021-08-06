# Data packing
=======
**TODO**: add an introductory and a summary videos.

This is a lab about data packing.

[<img src="https://drive.google.com/uc?export=view&id=16uvUgz327TXrysAf2HXYRe_KRBALHw2j" width="30%">](https://www.youtube.com/watch?v=-V-oIXrqA2s&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

You can decrease the memory traffic of the application if you pack the data more efficiently.
Some of the ways to do that include:

* Eliminate compiler-added padding.
* Use types that require less memory or less precision e.g. (int -> short, double -> float).
* Use bitfields to pack the data even further.

[<img src="https://drive.google.com/uc?export=view&id=12iavTVH9WUbb9BguLBLKe0QqdiPBMBiG" width="30%">](https://www.youtube.com/watch?v=ta096PQ6gTg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

# RESULTS
## Origin
```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1         477599 ns       477540 ns         1254
```

## Re-ordered fields to get rid off padding
```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1         455133 ns       455109 ns         1357
```

## Change data types base on the assumption that S.i is >= 0 and <= 100, re-order struct fields again
```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bench1         439567 ns       439517 ns         1567
```

** SOLUTION ***
* use pahole tool to re-organize `struct S` fields in packed order
* 
