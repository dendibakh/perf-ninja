
#include "solution.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

uint32_t original_solution(const char *file_name) {
  std::fstream file_stream{file_name};
  if (!file_stream.is_open())
    throw std::runtime_error{"The file could not be opened"};
  uint32_t crc = 0xff'ff'ff'ffu;
  char c;
  while (true) {
    file_stream.read(&c, 1);
    if (file_stream.eof())
      break;
    update_crc32(crc, static_cast<uint8_t>(c));
  }
  return crc ^ 0xff'ff'ff'ffu;
}

int main() {
  const auto original_result = original_solution(small_data);
  const auto result = solution(small_data);

  if (original_result != result) {
    std::cerr << "Validation Failed. Original result = " << original_result
              << "; Modified version returned = " << result << "\n";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
