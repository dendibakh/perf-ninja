#include "solution.hpp"

#include <fstream>
#include <stdexcept>

#include "MappedFile.hpp"

uint32_t solution(const char *file_name) {
  MappedFile file_stream{file_name};
  const auto file_data = file_stream.getContents();

  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  // Update the CRC32 value character by character
  for (const char c : file_data) {
    update_crc32(crc, static_cast<uint8_t>(c));
  }

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}
