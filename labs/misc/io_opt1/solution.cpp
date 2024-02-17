#include "solution.hpp"
#include "MappedFile.hpp"

#include <fstream>
#include <stdexcept>

#if 0 // mmap
uint32_t solution(const char *file_name) {
  auto mapped_file = MappedFile(std::string(file_name));
  auto content = mapped_file.getContents();
  uint32_t crc = 0xff'ff'ff'ff;
  for (auto& c : content) {
    update_crc32(crc, static_cast<uint8_t>(c));
  }
  // Invert the bits
  crc ^= 0xff'ff'ff'ff;
  return crc;
}
#endif

#if 1 //load chunks encapsulated
class ChunkLoader {
public:
  ChunkLoader(const char *file_name) : file_stream(file_name) {
    if (!file_stream.is_open())
        throw std::runtime_error{"The file could not be opened"};
   }
  uint8_t get() {
    if (cur_pos == symbols_read) {
      if (is_end()) {
        throw std::runtime_error{"End of file"};
      }
      file_stream.read(buf.data(), buf.size());
      symbols_read = file_stream.gcount();
      cur_pos = 0;
    }
    ++cur_pos;
    return static_cast<uint8_t>(buf[cur_pos-1]);
  }
  bool is_end() {
    return cur_pos == symbols_read && file_stream.eof();
  }
private:
  static constexpr int BUF_SIZE = 4000;
  std::array<char, BUF_SIZE> buf;
  std::fstream file_stream;
  int symbols_read = 0;
  int cur_pos = 0;
};

uint32_t solution(const char *file_name) {
  ChunkLoader chunk_loader{file_name};
  uint32_t crc = 0xff'ff'ff'ff;
  while(!chunk_loader.is_end()) {
    update_crc32(crc, chunk_loader.get());
  }
  crc ^= 0xff'ff'ff'ff;
  return crc;
}
#endif

#if 0 //load chunks simple
uint32_t solution(const char *file_name) {
  std::fstream file_stream{file_name};
  if (!file_stream.is_open())
    throw std::runtime_error{"The file could not be opened"};
  uint32_t crc = 0xff'ff'ff'ff;
  // Update the CRC32 value character by character
  constexpr int BUF_SIZE = 4000;
  std::array<char, BUF_SIZE> buf;
  while(!file_stream.eof()) {
    file_stream.read(buf.data(), buf.size());
    auto symbols_read = file_stream.gcount();
    for (int i = 0; i < symbols_read; ++i) {
      update_crc32(crc, static_cast<uint8_t>(buf[i]));
    }
  }
  // Invert the bits
  crc ^= 0xff'ff'ff'ff;
  return crc;
}
#endif

#if 0 //BASE_SOLUTION
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
#endif
