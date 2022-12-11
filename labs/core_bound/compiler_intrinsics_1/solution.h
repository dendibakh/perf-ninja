#pragma once
#include <cstdint>
#include <vector>

#if defined(__linux__) || defined(__linux) || defined(linux) ||                \
    defined(__gnu_linux__)
#define ON_LINUX
#elif defined(__APPLE__) && defined(__MACH__) && defined (__ARM_NEON__)
#define ON_MACOS
#elif defined(_WIN32) || defined(_WIN64)
#define ON_WINDOWS
#endif

using InputVector = std::vector<uint8_t>;
using OutputVector = std::vector<uint16_t>;
constexpr uint8_t radius = 13; // assume diameter (2 * radius + 1) to be less
                               // than 256 so results fits in uint16_t

void init(InputVector &data);
void zero(OutputVector &data, std::size_t size);
void imageSmoothing(const InputVector &inA, uint8_t radius,
                    OutputVector &outResult);
