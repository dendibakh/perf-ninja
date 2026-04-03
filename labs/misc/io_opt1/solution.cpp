#include "solution.hpp"
#include "MappedFile.hpp"

#include <fstream>
#include <stdexcept>

uint32_t solution(const char *file_name) {

  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  MappedFile f(file_name);
  auto sv = f.getContents();
  for (auto c : sv) {
    update_crc32(crc, static_cast<uint8_t>(c));
  }

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}
