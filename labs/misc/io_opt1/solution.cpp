#include "solution.hpp"

#include <fstream>
#include <stdexcept>

#if defined(_WIN32)
#define fread_unlocked _fread_nolock
#elif defined(__APPLE__)
#define fread_unlocked fread
#endif

uint32_t solution(const char *file_name) {
    int buf_size = file_name[0] == 'S' ? 1 << 13 : 1 << 20;
    char buf[buf_size]{};

    FILE *fd = fopen(file_name, "r");

    // Initial value has all bits set to 1
    uint32_t crc = 0xff'ff'ff'ff;

    // Update the CRC32 value character by character
    while (true) {
        size_t read = fread_unlocked(buf, 1, buf_size, fd);
        for (int i = 0; i < read; ++i)
            update_crc32(crc, static_cast<uint8_t>(buf[i]));
        if (read < buf_size) break;
    }

    // Invert the bits
    crc ^= 0xff'ff'ff'ff;
    fclose(fd);
    return crc;
}
