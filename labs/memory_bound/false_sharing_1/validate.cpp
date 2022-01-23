#include "solution.hpp"

#include <iostream>
#include <numeric>
#include <thread>

size_t original_solution(const std::vector<uint32_t> &data) {
  size_t value = 0;

  for (int i = 0; i < data.size(); i++) {
    auto item = data[i];
    item += 1000;
    item ^= 0xADEDAE;
    item |= (item >> 24);

    value += item % 13;
  }

  return value;
}

int main() {
  const auto size = 16 * 1024 * 1024;

  std::vector<uint32_t> data;
  data.reserve(size);

  for (int i = 0; i < size; i++) {
    data.push_back(i);
  }

  auto original_result = original_solution(data);

  // Use thread count from 1 to <number of HW threads>
  size_t max_threads = std::thread::hardware_concurrency();
  std::vector<int> threads(max_threads);
  std::iota(threads.begin(), threads.end(), 1);

  for (auto thread_count : threads) {
    auto result = solution(data, thread_count);
    if (original_result != result) {
      std::cerr << "Validation Failed for " << thread_count
                << " thread(s). Original result = " << original_result
                << "; Modified version returned = " << result << "\n";
      return 1;
    }
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
