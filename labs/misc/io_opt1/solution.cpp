#include "solution.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

uint32_t solution(const char *file_name) {
  // Initial value has all bits set to 1
  uint32_t crc = 0xff'ff'ff'ff;

  int fd = open(file_name, O_RDONLY);
  if (fd == -1) {
    std::cerr << "ZHOPA" << std::endl;
    exit(1);
  }
  struct stat st;
  if (fstat(fd, &st) == -1) {
    std::cerr << "ZHOPA2" << std::endl;
    exit(1);
  }

  int fsz = st.st_size;
  char* addr = (char*)mmap(NULL, fsz, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    std::cerr << "ZHOPA3" << std::endl;
    exit(1);
  }
  for (int i = 0; i < fsz; ++i) {
    update_crc32(crc, static_cast<uint8_t>(addr[i]));
  }

  // Invert the bits
  crc ^= 0xff'ff'ff'ff;

  return crc;
}
