#include "solution.hpp"

#include <fstream>
#include <stdexcept>

uint32_t solution(const char *file_name) {
  std::fstream file_stream{file_name};
  if (!file_stream.is_open())
    throw std::runtime_error{"The file could not be opened"};

  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  // Update the CRC32 value character by character
  char c;
  while (true) {
    file_stream.read(&c, 1);
    if (file_stream.eof())
      break;
    update_crc32(crc, static_cast<uint8_t>(c));
  }

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}
