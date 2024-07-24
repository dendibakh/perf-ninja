#include "solution.hpp"

#include <fstream>
#include <stdexcept>

#ifdef __APPLE__
char buf[1 << 28]{};
#else
char buf[1 << 20]{};
#endif

uint32_t solution(const char *file_name) {
    FILE *fd = fopen(file_name, "r");
    if (fd == nullptr) {
        throw std::runtime_error{"The file could not be opened"};
    }
    // Initial value has all bits set to 1
    uint32_t crc = 0xff'ff'ff'ff;

    // Update the CRC32 value character by character
    while (true) {
        size_t read = fread_unlocked(buf, 1, sizeof(buf), fd);
        for (int i = 0; i < read; ++i)
            update_crc32(crc, static_cast<uint8_t>(buf[i]));
        if (read < sizeof(buf)) break;
    }

    // Invert the bits
    crc ^= 0xff'ff'ff'ff;
    fclose(fd);
    return crc;
}
