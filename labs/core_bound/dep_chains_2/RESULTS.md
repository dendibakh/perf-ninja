# Dependency Chains 2 — Results

## In one paragraph
Simulate 1 000 particles moving randomly on a 2-D plane for 1 000 steps. Each step calls a pseudo-random number generator (RNG) to pick a direction, then updates the particle's position. The RNG uses a single global state variable, so every call to the RNG depends on the result of the previous call — a hidden serial dependency that forces the entire simulation to run one RNG call at a time.

## The mental picture
Imagine rolling a single dice to move all 1 000 pieces on a board game: you roll for piece 1, record the result, roll for piece 2 (using the result of roll 1 as your starting seed), and so on. You cannot roll for piece 2 until piece 1's roll is finished. The dice rolls are the bottleneck, not the board moves.

The fix: give each particle its *own* dice (its own RNG state). Now all 1 000 dice can be thrown conceptually in parallel — the CPU can pipeline or overlap the arithmetic, and IPC (instructions per cycle) shoots up.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **16 ms** for 1 000 particles × 1 000 steps |
| Per-step cost | ≈ 16 ns/step |
| Stability (CV) | 0.50 % |

## What the optimization looks like
Give each particle its own RNG seed (stored per-particle). The RNG calls for different particles become independent, breaking the global-state chain and letting the CPU overlap arithmetic across particles.

## Pattern → [PATTERNS.md: Serial dependency chains](../../PATTERNS.md#serial-dependency-chains)
