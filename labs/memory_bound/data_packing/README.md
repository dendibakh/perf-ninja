**NOTE: this lab is currently broken.**
**After migrating to a new compiler version the speedup is no longer measurable consistently.**
**You can still try to solve it to learn the concept, but the result is not guaranteed.**

# Data packing

This is a lab about data packing.

[<img src="../../../img/DataPacking1Intro.png">](https://www.youtube.com/watch?v=-V-oIXrqA2s&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

You can decrease the memory traffic of the application if you pack the data more efficiently.
Some of the ways to do that include:

* Eliminate compiler-added padding.
* Use types that require less memory or less precision e.g. (int -> short, double -> float).
* Use bitfields to pack the data even further.

[<img src="../../../img/DataPacking1Summary.png">](https://www.youtube.com/watch?v=ta096PQ6gTg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)
