# Data packing

This is a lab about data packing.

[<img src="../../../img/DataPacking1Intro.png">](https://www.youtube.com/watch?v=-V-oIXrqA2s&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

You can decrease the memory traffic of the application if you pack the data more efficiently.
Some of the ways to do that include:

* Eliminate compiler-added padding.
* Use types that require less memory or less precision e.g. (int -> short, double -> float).
* Use bitfields to pack the data even further.

**Note:** Data Packing Summary video mentions branch mispredictions as a primary bottleneck for this lab. This is no longer true since the main source of branch mispredictions (std::sort) was replaced by counting sort.

[<img src="../../../img/DataPacking1Summary.png">](https://www.youtube.com/watch?v=ta096PQ6gTg&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)
