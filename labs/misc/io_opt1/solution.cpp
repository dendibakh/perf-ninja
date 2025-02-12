#include "solution.hpp"
#include "MappedFile.hpp"

#include <stdexcept>

uint32_t solution(const char *file_name) {
  auto m{MappedFile(file_name)};
  auto content{m.getContents()};

  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  // Update the CRC32 value character by character
  for (char c : content) {
    update_crc32(crc, static_cast<uint8_t>(c));
  }

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}
