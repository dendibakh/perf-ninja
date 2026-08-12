# Warmup — Results

## In one paragraph
This is the introductory lab: a minimal workload designed to familiarise you with the build system, benchmark harness, and validation workflow before you tackle real performance problems. The workload itself is intentionally trivial.

## Numbers
| Metric | Value |
|---|---|
| Validation | ✅ |
| Baseline mean | **47 ns** per call |
| Stability (CV) | 2.57 % |

The CV here (2.57%) is slightly higher than heavier labs because on such short timescales (~47 ns) even a single cache miss is proportionally large noise. That is normal and expected for a trivial kernel.

## Purpose
Use this lab to verify your environment is set up correctly:
- CMake finds the benchmark library.
- `validateLab` prints "Validation Successful."
- `./lab` produces numbers in the right ballpark.

If this lab doesn't build or validate, fix the environment before moving on — all 25 other labs use the same CMake skeleton.

## Pattern
No specific performance pattern — this is the "hello world" of the course.
