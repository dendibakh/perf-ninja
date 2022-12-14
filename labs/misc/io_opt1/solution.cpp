#include "solution.hpp"
#include "MappedFile.hpp"
#include <fstream>
#include <algorithm>
#include <stdexcept>

uint32_t solution(const char *file_name) {
  // Map the file into memory
  MappedFile file{file_name};

  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  // Update the CRC32 value with an algorithm
  const auto& str = file.getContents();
  std::for_each(str.begin(), str.end(), [&crc](char c){
    update_crc32(crc, static_cast<uint8_t>(c));
  });

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}