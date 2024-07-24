#include "solution.hpp"

#include <fstream>
#include <stdexcept>

// #if ON_WINDOwS
// #define fread_unlocked _fread_nolock
// #elif ON_MACOS
// #define fread_unlocked fread
// #endif

// char buf[1 << 24]{};
uint32_t solution(const char *file_name) {
    FILE *fd = fopen(file_name, "r");
    if (fd == nullptr) {
        throw std::runtime_error{"The file could not be opened"};
    }
    // Initial value has all bits set to 1
    uint32_t crc = 0xff'ff'ff'ff;

    // Update the CRC32 value character by character
    int c;
    while ((c = fgetc(fd)) != EOF) {
        // size_t read = fread_unlocked(buf, 1, sizeof(buf), fd);
        // for (int i = 0; i < read; ++i)
        //     update_crc32(crc, static_cast<uint8_t>(buf[i]));
        update_crc32(crc, static_cast<uint8_t>(c));
    }

    // Invert the bits
    crc ^= 0xff'ff'ff'ff;
    fclose(fd);
    return crc;
}
