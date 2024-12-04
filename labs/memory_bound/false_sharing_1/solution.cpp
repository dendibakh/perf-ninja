#include "solution.hpp"
#include <atomic>
#include <cstring>

#include <vector>

std::size_t solution(const std::vector<uint32_t> &data, int thread_count) {
  // Using std::atomic counters to disallow compiler to promote `target`
  // memory location into a register. This way we ensure that the store
  // to `target` stays inside the loop.
  struct Accumulator {
    std::atomic<uint32_t> value = 0;
  };
  std::vector<Accumulator> accumulators(thread_count);


  for(auto& target : accumulators)
  {
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
