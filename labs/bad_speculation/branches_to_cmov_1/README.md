**TODO**: add an introductory and a summary videos.

This is the good old game of life. The program takes 10 randomly generated 1024x1024 grids (boards) and simulates the next 10 game of life rounds. It first simulates 10 rounds for the first board, then goes off to the second board, and so on.

As written, the program experiences many branch mispredictions. Your job is to find where they happen and replace them with predicate instructions. On x86 you should see `cmov` instructions (hint: use `__builtin_unpredictable`, make sure you have Clang-17 or later version installed). On ARM you should see `csel` (conditional select) instructions. It is a good idea to experiment with the code in godbolt.org before you start modifying the actual code.

Bonus question: when you fix the main source of mispredictions, there are still many branches left in the hot loop nest. How can you get rid of them?
