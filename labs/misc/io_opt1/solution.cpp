#include "solution.hpp"
#include <array>
#include <fstream>
#include <stdexcept>

uint32_t solution(const char *file_name) {
  std::fstream file_stream{file_name};
  if (!file_stream.is_open())
    throw std::runtime_error{"The file could not be opened"};

  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  static constexpr int N = 4096;
  std::array<char, N> buffer;
  
  while (true) {
    file_stream.read(buffer.data(), N);
    auto bytes_read = file_stream.gcount();
    for (std::streamsize i = 0; i < bytes_read; i++)
      update_crc32(crc, static_cast<uint8_t>(buffer[i]));
    if (file_stream.eof())
      break;
  }

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}
