#include "solution.hpp"

#include <fstream>
#include <stdexcept>

#include "MappedFile.hpp"

uint32_t solution(const char *file_name) {
  MappedFile mapped_file(file_name);

  auto contents = mapped_file.getContents();

  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  // Update the CRC32 value character by character
  for (size_t i = 0; i < contents.size(); i++) {
    update_crc32(crc, static_cast<uint8_t>(contents[i]));
  }

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}
