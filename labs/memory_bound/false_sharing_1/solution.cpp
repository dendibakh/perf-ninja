#include "solution.hpp"
#include <atomic>
#include <cstring>
#include <omp.h>
#include <thread>
#include <iostream>
#include <vector>

std::size_t solution(const std::vector<uint32_t> &data, int thread_count) {
  // Using std::atomic counters to disallow compiler to promote `target`
  // memory location into a register. This way we ensure that the store
  // to `target` stays inside the loop.
  struct Accumulator {
    std::atomic<uint32_t> value = 0;
  };
  std::vector<Accumulator> accumulators(thread_count);
  std::vector<std::thread> threads;

  const int datasz = data.size();
  const int step = datasz / thread_count + 1;
  for (int i = 0; i < thread_count; ++i) {
    threads.emplace_back([i, step, &data, &accumulators, datasz]() {
      int endd = std::min(datasz, (i + 1) * step);
      for (int j = i * step; j < endd; ++j) {
        auto item = data[j];
        item += 1000;
        item ^= 0xADEDAE;
        item |= (item >> 24);

        // Write result to accumulator
        accumulators[i].value += item % 13;
      }
    });
  }
  std::size_t result = 0;
  for (int i = 0; i < thread_count; ++i) {
    threads[i].join();
    result += accumulators[i].value;
  }
  return result;
}
