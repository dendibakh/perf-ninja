
#include <array>
#include <cstdint>
#include <cstring>

// Assume this constant never changes
constexpr int N = 400;

// Square matrix 400 x 400
using Matrix = std::array<std::array<float, N>, N>;

inline void zero(Matrix &result) { memset(&result, 0x0, sizeof(result)); }
void identity(Matrix &result);
void multiply(Matrix &result, const Matrix &a, const Matrix &b);
Matrix power(const Matrix &input, const uint32_t k);

void init(Matrix &matrix);
