**TODO**: add an introductory and a summary videos.

This small program simulates the random particle movement. We have 1000 particles moving on a 2D surface without constraints. It means that there are no bounds, they can move as far from their initial coordinates as they want. Each particle has initial x and y coordinates in the range [-1000,1000] and a constant speed in the range [0;1]. The program simulates 1000 steps of particle movement.

To validate the simulation (final positions of the particles), we use a deterministic random number generator (fake) that uses a global state and thus always generates the same sequence of numbers.

There is one very nasty performance problem that doesn't allow us to run the simulation fast. Can you find a dependency chain in the code and fix it?

Note: your solution is allowed to be not functionally equivalent to the original program if validation still passes. For example, if an RNG will generate a different sequence of random numbers than before - that's OK. Users would not be able to tell the difference since the motion of particles is random anyway.

## Hints

<details>
<summary><b>Hint 1:</b></summary>

This is a dependency chain problem. The first thing you should try to do is identify why given iterations of the loop in
`randomParticleMotion` in `solution.hpp` are not independent. Is there a value used in iteration N that depends on what
is calculated in iteration N-1?

</details>

<br>

<details>
<summary><b>Hint 2:</b></summary>

Take a look at the implementation of `XorShift32::gen()`. What do you notice about the return value of this function?

</details>

<br>

<details>
<summary><b>Hint 3:</b></summary>

The trick to improving performance in dependency chain problems (where the dependency cannot be changed) is to try
to handle "chunks" of the chain per iteration, rather than a single node. See if you can find an optimal solution for
your machine.

</details>

<br>

<details>
<summary><b>Worked Solution:</b></summary>

## Background:

The program in this lab simulates random particle motion. The relevant code loop is here in `solution.hpp`:

```c++
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
  RNG rng(seed);  
  for (int i = 0; i < STEPS; i++)
    for (auto &p : particles) {
      uint32_t angle = rng.gen();
      float angle_rad = angle * DEGREE_TO_RADIAN;
      p.x += cosine(angle_rad) * p.velocity;
      p.y += sine(angle_rad) * p.velocity;
    }
}
```

We see a random number generator is used, and it can be a templated type (`RNG`). The lab uses the following class for
generating the next random number with the function `gen()`:

```c++
struct XorShift32 {
  uint32_t val;
  XorShift32 (uint32_t seed) : val(seed) {}

  uint32_t gen() {
    val ^= (val << 13);
    val ^= (val >> 17);
    val ^= (val << 5);
    return val;
  }
};
```

If we analyse the loop in `randomParticleMotion` carefully, we will notice that the implementation of `XorShift32` has
introduced a major dependency chain into our code. The implementation of `XorShift32` is not stateless; the return value
of `gen()` relies on the current value of `XorShift32::val`. If we then look back at our loop, we see that future
iterations cannot be performed in advance by the CPU as the future value of `gen()` cannot be known until all the
iterations before it have been completed.

Here are benchmark results for the original code:

```
----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
bench1/iterations:200       19.4 ms         19.4 ms          200
```

### Solution

Since the dependency chain of `gen()` is only six bitwise instructions, we can simply unroll the loop manually to
speed up the processing:

```c++
template <class RNG>
void randomParticleMotion(std::vector<Particle>& particles, uint32_t seed) {
  RNG rng(seed);
  for (int i = 0; i < STEPS; i++) {
    auto sz = particles.size();
    for (int j = 0; j + 1 < sz; j += 2) {
      uint32_t angle1 = rng.gen();
      float angle_rad = angle1 * DEGREE_TO_RADIAN;
      particles[j].x += cosine(angle_rad) * particles[j].velocity;
      particles[j].y += sine(angle_rad) * particles[j].velocity;
      
      uint32_t angle2 = rng.gen();
      float angle_rad2 = angle2 * DEGREE_TO_RADIAN;
      particles[j+1].x += cosine(angle_rad2) * particles[j+1].velocity;
      particles[j+1].y += sine(angle_rad2) * particles[j+1].velocity;
    }
    if (sz % 2) {
      angle = rng.gen();
      float angle_rad = angle * DEGREE_TO_RADIAN;
      particles[sz-1].x += cosine(angle_rad) * particles[sz-1].velocity;
      particles[sz-1].y += sine(angle_rad) * particles[sz-1].velocity;
    }
  }
}
```

### Bonus Solution

In the above solution, we could manually unroll the loop because the dependency chains were fairly small. The original
dependency chain in `XorShift32::gen()` was only six instructions long (a few bitwise operations). In a situation where
the chain is a lot more involved, we cannot benefit from unrolling the loop, because the total number of instructions
would exceed the number that the CPU is physically able to see as future, upcoming instructions (in its _reservation station_).

In such a scenario, we would need to rewrite `XorShift32` to  _interleaving_ dependency chains together in `gen()`.

The logic is as follows:

Let's decide we want to produce two chains. In this scenario, we would be producing _two_ random numbers at once. We
essentially take every line of code that is currently in `gen()` and duplicate it for the second random number. However,
these lines must be _interleaved_:

```c++
int val1 = 0, int val2 = 0;
...

// WRONG - all the operations done on `val1` come before `val2`
val1 = doSomething(val1);
val1 = doSomethingElse(val1);
val2 = doSomething(val2);
val2 = doSomethingElse(val2);
...

// CORRECT - the operations on `val1` and `val2` are interleaved
val1 = doSomething(val1);
val2 = doSomething(val2);
val1 = doSomethingElse(val1);
val2 = doSomethingElse(val2);
```

Here's how our random number generator could look after this change:

```c++
/// Lightweight Pair class - the overheads of std::pair are unnecessary here
template<typename First, typename Second>
struct Pair {
  First first;
  Second second;
};

struct XorShift32 {
  Pair<uint32_t, uint32_t> val;
  XorShift32 (uint32_t seed) {
      val.first = seed;
      uint32_t second = val.first;
      second ^= (second << 13);
      second ^= (second >> 17);
      second ^= (second << 5);
      val.second = second;
  }

  auto gen() {
    val.first ^= (val.first << 13);
    val.second ^= (val.second << 13);

    val.first ^= (val.first >> 17);
    val.second ^= (val.second >> 17);

    val.first ^= (val.first << 5);
    val.second ^= (val.second << 5);
    return val;
  }
};
```

We can rewrite our loop in `randomParticleMotion` as follows:

```c++
template <class RNG>
void randomParticleMotion(std::vector<Particle>& particles, uint32_t seed) {
  RNG rng(seed);
  for (int i = 0; i < STEPS; i++) {
    auto sz = particles.size();
    for (int j = 0; j + 1 < sz; j += 2) {
      Pair<uint32_t, uint32_t> angle = rng.gen();
      float angle_rad = angle.first * DEGREE_TO_RADIAN;
      particles[j].x += cosine(angle_rad) * particles[j].velocity;
      particles[j].y += sine(angle_rad) * particles[j].velocity;
      
      float angle_rad2 = angle.second * DEGREE_TO_RADIAN;
      particles[j+1].x += cosine(angle_rad2) * particles[j+1].velocity;
      particles[j+1].y += sine(angle_rad2) * particles[j+1].velocity;
    }
    if (sz % 2) {
      Pair<uint32_t, uint32_t> angle = rng.gen();
      float angle_rad = angle.first * DEGREE_TO_RADIAN;
      particles[sz-1].x += cosine(angle_rad) * particles[sz-1].velocity;
      particles[sz-1].y += sine(angle_rad) * particles[sz-1].velocity;
    }
  }
}
```

In this scenario, this method does not bring about much of a performance benefit at all, but that is expected as the
dependency chain of `rng.gen()` is extremely short. However, in most code affected by dependency chains, the chains will
not be so trivial, so knowing this bonus technique will be necessary in most real situations!
</details>

<br>

<details>
<summary><b>Code for Solution (Link):</b></summary>

[Link to Solution](https://github.com/dendibakh/perf-ninja/blob/golden/labs/core_bound/dep_chains_2/solution.cpp)

</details>