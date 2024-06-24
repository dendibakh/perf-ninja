**TODO**: add an introductory and a summary videos.

This small program simulates the random particle movement. We have 1000 particles moving on a 2D surface without constraints. It means that there are no bounds, they can move as far from their initial coordinates as they want. Each particle has initial x and y coordinates in the range [-1000,1000] and a constant speed in the range [0;1]. The program simulates 1000 steps of particle movement.

To validate the simulation (final positions of the particles), we use a deterministic random number generator (fake) that uses a global state and thus always generates the same sequence of numbers.

There is one very nasty performance problem that doesn't allow us to run the simulation fast. Can you find a dependency chain in the code and fix it?

Note: your solution is allowed to be not functionally equivalent to the original program if validation still passes. For example, if an RNG will generate a different sequence of random numbers than before - that's OK. Users would not be able to tell the difference since the motion of particles is random anyway.
