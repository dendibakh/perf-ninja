#include "solution.hpp"

#include <fstream>
#include <stdexcept>
FILE *fd;
constexpr bool likely(bool x) {
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_expect(x, 1);
#else
    return x;
#endif
}
constexpr bool unlikely(bool x) { return !likely(!x); }
char buf[1 << 16]{};
size_t bc = 0, be = 0;
bool last_read = false;
int gc() {
    if (likely(bc < be)) {
        return buf[bc++];
    } else {
        if (last_read) return EOF;
        buf[0] = 0, bc = 0;
        be = fread(buf, 1, sizeof(buf), fd);
        if (unlikely(feof(fd))) {
            last_read = true;
            buf[be] = EOF;
        }
        return buf[bc++];
    }
}

uint32_t solution(const char *file_name) {
    fd = fopen(file_name, "r");
    if (fd == nullptr) {
        throw std::runtime_error{"The file could not be opened"};
    }
    // Initial value has all bits set to 1
    uint32_t crc = 0xff'ff'ff'ff;

    // Update the CRC32 value character by character
    int c;
    while ((c = gc()) != EOF) {

        update_crc32(crc, static_cast<uint8_t>(c));
    }

    // Invert the bits
    crc ^= 0xff'ff'ff'ff;
    fclose(fd);
    return crc;
}
