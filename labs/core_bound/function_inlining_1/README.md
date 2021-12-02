[<img src="https://drive.google.com/uc?export=view&id=1CdVTip7DNJKmvouo2pfmog0OWl0km9p_" width="30%">](https://www.youtube.com/watch?v=fp1_e3rjZQs&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d&index=12)

This is a lab about [function inlining](https://en.wikipedia.org/wiki/Inline_expansion) to speed up sorting.

Function inlining is a transformation that replaces a call to a function `F` with the body for `F` specialized with the actual arguments of the call. Inlining is one of the most important compiler optimizations, not only because it eliminates the overhead of calling a function (prologue and epilogue), but also it enables other optimizations.

Whenever you find in a performance profile a function with hot prologue and epilogue, consider such function as one of the potential candidates for being inlined. In this lab assignment you will practice fixing such performance issues.

[<img src="https://drive.google.com/uc?export=view&id=1lOajRZmKvGDuU2bEm24Iu76zirPEdw1a" width="30%">](https://www.youtube.com/watch?v=qlFUV0FjpPQ&list=PLRWO2AL1QAV6bJAU2kgB4xfodGID43Y5d&index=12)