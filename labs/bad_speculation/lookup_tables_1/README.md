[<img src="../../../img/LookupTables1.png">](https://www.youtube.com/watch?v=bhz4t5QYApE&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

Welcome to the next lab assignment, where we will fight branch mispredictions by replacing them with lookup tables. The code in this lab assignment maps values from `[0;150]` into buckets, which involves a lot of comparisons, and so, branches. To solve this assignment you need to figure out a way how to replace *all* hard-to-predict branches.

Bonus question: how would you solve it if the range of possible values will be bigger, say `[0;10000]`?
