#include "solution.hpp"

#include <fstream>
#include <stdexcept>
FILE *fd;
[[maybe_unused]] constexpr bool assume(bool x) {
#if defined(__clang__) || defined(__GNUC__)
    if (!x) __builtin_unreachable();
#endif
    return x;
}
constexpr bool likely(bool x) {
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_expect(x, 1);
#else
    return x;
#endif
}
constexpr bool unlikely(bool x) { return !likely(!x); }
namespace _gc_data {
char buf[1 << 16]{};
size_t bc = 0, be = 0;
bool last_read = false;
} // namespace _gc_data
[[maybe_unused]] void un_gc() { --_gc_data::bc; }
int gc() {
    if (likely(_gc_data::bc < _gc_data::be)) {
        return _gc_data::buf[_gc_data::bc++];
    } else {
        if (_gc_data::last_read) return EOF;
        _gc_data::buf[0] = 0, _gc_data::bc = 0;
        _gc_data::be = fread_unlocked(_gc_data::buf, 1, sizeof(_gc_data::buf), fd);
        if (unlikely(feof(fd))) {
            _gc_data::last_read = true;
            _gc_data::buf[_gc_data::be] = EOF;
        }
        return _gc_data::buf[_gc_data::bc++];
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
    char c;
    while ((c = gc()) != EOF) {

        update_crc32(crc, static_cast<uint8_t>(c));
    }

    // Invert the bits
    crc ^= 0xff'ff'ff'ff;
    fclose(fd);
    return crc;
}
