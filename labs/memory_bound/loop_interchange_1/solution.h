
#include <array>
#include <cstdint>

// Assume this constant never changes
constexpr int N = 400;
constexpr int i_stride = 80;
constexpr int j_stride = 80;
constexpr int k_stride = 80;

// Square matrix 400 x 400
using Matrix = std::array<std::array<float, N>, N>;

void zero(Matrix &result);
void identity(Matrix &result);
void multiply(Matrix &result, const Matrix &a, const Matrix &b);
Matrix power(const Matrix &input, const uint32_t k);

void init(Matrix &matrix);
