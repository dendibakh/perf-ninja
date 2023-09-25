# Describe the benchmark

This is the good old game of life. The program takes 10 randomly generated 1024x1024 grids (boards) and simulates the next 10 game of life rounds. Right now the program is very sequential, i.e. it does 10 rounds for the first board, then goes off to the second board, and so on.

```
game1
  |
  -sim1 -> ...  simN
                   |
                   game2
                     |
                     -sim1 -> ... simN

------------------------------------>
                                 time
```

Think about how you can process two games in parallel.

Further questions:
- Change the size of the grid to 16x16. Observe what happens with the benchmark. Why do you think it happens?
- What if you start processing 3 or 4 boards in parallel? Observe what happens with the benchmark. What limitations do you see?
