#include "solution.hpp"
#include <atomic>
#include <cstring>
#include <new>
#include <omp.h>
#include <vector>

#ifdef __cpp_lib_hardware_interference_size
    using std::hardware_destructive_interference_size;
#else
    constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

std::size_t solution(const std::vector<uint32_t> &data, int thread_count) {
  // Using std::atomic counters to disallow compiler to promote `target`
  // memory location into a register. This way we ensure that the store
  // to `target` stays inside the loop.


#ifndef SOLUTION
  #define SOLUTION 1
#endif

#if SOLUTION == 0
  // Baseline.
  struct Accumulator {
    std::atomic<uint32_t> value = 0;
  };

#elif SOLUTION == 1
// My solution.

  struct alignas(hardware_destructive_interference_size) Accumulator {
    std::atomic<uint32_t> value = 0;
  };

#elif SOLUTION == 2
// Facit.

  #define CACHELINE_ALIGN alignas(64)
  struct CACHELINE_ALIGN Accumulator {
    std::atomic<uint32_t> value = 0;
  };

#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif

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
