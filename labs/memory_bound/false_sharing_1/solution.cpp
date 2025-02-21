#include "solution.hpp"
#include <atomic>
#include <cstring>
#include <omp.h>
#include <vector>
#define SOLUTION
#ifndef SOLUTION
std::size_t solution(const std::vector<uint32_t> &data, int thread_count) {
  // Using std::atomic counters to disallow compiler to promote `target`
  // memory location into a register. This way we ensure that the store
  // to `target` stays inside the loop.
  struct Accumulator {
    std::atomic<uint32_t> value = 0;
  };
  std::vector<Accumulator> accumulators(thread_count);

#pragma omp parallel num_threads(thread_count) default(none)                   \
    shared(accumulators, data)
  {
    int target_index = omp_get_thread_num();
    auto &target = accumulators[target_index];

#pragma omp for
    for (int i = 0; i < data.size(); i++) {
      // Perform computation on each input
      auto item = data[i];
      item += 1000;
      item ^= 0xADEDAE;
      item |= (item >> 24);

      // Write result to accumulator
      target.value += item % 13;
    }
  }

  std::size_t result = 0;
  for (const auto &accumulator : accumulators) {
    result += accumulator.value;
  }
  return result;
}
#else

#if defined(__APPLE__) && defined(__MACH__)
  // In Apple processors (such as M1, M2 and later), L2 cache operates on 128B cache lines.
  #define ON_MACOS
  #define CACHELINE_SIZE 128
#else
  #define CACHELINE_SIZE 64
#endif

std::size_t solution(const std::vector<uint32_t> &data, int thread_count) {
  // Using std::atomic counters to disallow compiler to promote `target`
  // memory location into a register. This way we ensure that the store
  // to `target` stays inside the loop.
  struct alignas(CACHELINE_SIZE) Accumulator {
    std::atomic<uint32_t> value = 0;
  };
  std::vector<Accumulator> accumulators(thread_count);

#pragma omp parallel num_threads(thread_count) default(none)                   \
    shared(accumulators, data)
  {
    int target_index = omp_get_thread_num();
    auto &target = accumulators[target_index];

#pragma omp for
    for (int i = 0; i < data.size(); i++) {
      // Perform computation on each input
      auto item = data[i];
      item += 1000;
      item ^= 0xADEDAE;
      item |= (item >> 24);

      // Write result to accumulator
      target.value += item % 13;
    }
  }

  std::size_t result = 0;
  for (const auto &accumulator : accumulators) {
    result += accumulator.value;
  }
  return result;
}
#endif