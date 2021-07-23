
#include <cstdint>
#include <vector>

using InputVector = std::vector<uint8_t>;
using OutputVector = std::vector<uint16_t>;
constexpr uint8_t radius = 13; // assume diameter (2 * radius + 1) to be less
                               // than 256 so results fits in uint16_t

void init(InputVector &data);
void zero(OutputVector &data, std::size_t size);
void imageSmoothing(const InputVector &inA, uint8_t radius,
                    OutputVector &outResult);
