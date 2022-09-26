[<img src="../../../img/Vectorization2_button.png">](https://www.youtube.com/watch?v=m4SWal8EAgM&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d)

This is a second lab about [auto vectorization](https://llvm.org/docs/Vectorizers.html). The subject of this lab assignment is a part of a checksum algorithm from the 80s, which has risen from the popularity of the Internet and [accompanying needs to validate transmitted packets](https://www.alpharithms.com/internet-checksum-calculation-steps-044921/). Even the problem is old, similar issues may exist nowadays in production code.

Modern compilers handle simple loops very well, including horizontal additions. In this lab computations inside the loop are slightly more difficult: we do an "add carry" operation. Some compilers recognize "add carry" and others don't. The [carry flag](https://en.wikipedia.org/wiki/Binary_number#Binary_arithmetic) is still a dark area in C++ while it exists more than 40 years. In this lab assignment, you will practice fixing auto-vectorization, which will improve performance significantly.

Hint: the [RFC 1071](http://www.faqs.org/rfcs/rfc1071.html) paper in the section "2. Calculating the Checksum" describes possible techniques to speed up this assignment. Also, clang can help to find [causes](https://llvm.org/docs/Vectorizers.html#diagnostics) of bad performance.
