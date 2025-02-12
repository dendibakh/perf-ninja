#include "solution.hpp"

#include <fstream>
#include <stdexcept>

#define N_READ 1024

uint32_t solution(const char *file_name) {
  std::ifstream file_stream{file_name};
  if (!file_stream.is_open())
    throw std::runtime_error{"The file could not be opened"};

  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  // Update the CRC32 value character by character
  char c[N_READ];
  while (true) {
    file_stream.read(c, N_READ);
    int count = file_stream.gcount();
    for (int i{}; i < count; ++i) {
    	update_crc32(crc, static_cast<uint8_t>(c[i]));
    }
    if (count < N_READ) break;
  }

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}
