[<img src="../../../img/CompilerIntrinsics1-Intro.png">](https://www.youtube.com/watch?v=mlXw_qYRi78&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d&index=12)

This is a lab about using [compiler intrinsics](https://en.wikipedia.org/wiki/Intrinsic_function) to speed up parts of the code, where compilers fail to generate optimal code.

The kernel in this lab assignment is a part of the Average ImageSmoothing algorithm, which is reduced to 1 dimension and lacks division part. The algorithm uses sliding window approach to compute a sum in the subrange [-radius .. +radius]. It is a very fast approach compared to a classical Gaussian blur.

[<img src="../../../img/CompilerIntrinsics1-Summary.png">](https://www.youtube.com/watch?v=fP6Rhwf3rEs&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d&index=12)

Author: @adamf88.