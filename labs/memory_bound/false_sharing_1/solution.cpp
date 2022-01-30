#include "solution.hpp"
#include <atomic>
#include <cstring>
#include <omp.h>
#include <vector>

size_t solution(const std::vector<uint32_t> &data, int thread_count) {
	// [Problem 1] False sharing.
  constexpr uint32_t CACHE_LINE = 64;
  
  // Using std::atomic counters to disallow compiler to promote `target`
  // memory location into a register. This way we ensure that the store
  // to `target` stays inside the loop.
  struct Accumulator {
    std::atomic<uint32_t> value = 0;

  private:
	uint8_t padding[CACHE_LINE];
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

	  // [Problem 2] Expensive lock instruction.
	  // Operator += and fetch_add() are RMW operations that lock the bus. However, only a single
	  // thread does modify the given target and there is no need for such an expensive lock instruction.
	  const auto new_value = target.value.load(std::memory_order_relaxed) + (item % 13);
      target.value.store(new_value, std::memory_order_relaxed);
    }
  }

  std::size_t result = 0;
  for (const auto &accumulator : accumulators) {
    result += accumulator.value;
  }
  return result;
}
