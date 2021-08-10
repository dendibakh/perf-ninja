
#include "solution.h"
#include <cstdint>

class MatrixView{
public:
  // NOLINTNEXTLINE: implicit conversion is desired
  MatrixView(Matrix& m): m{&m}{}

  const Matrix::value_type& operator[](const std::size_t index) const
  {
    return (*m)[index];
  }

  Matrix::value_type& operator[](const std::size_t index)
  {
    return (*m)[index];
  }

  Matrix& raw()
  {
    return *m;
  }

  friend void swap(MatrixView& lhs, MatrixView& rhs)
  {
    std::swap(lhs.m, rhs.m);
  }

private:
  Matrix* m{};
};

// Make zero matrix
void zero(Matrix &result) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      result[i][j] = 0;
    }
  }
}

// Make identity matrix
void identity(Matrix &result) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      result[i][j] = 0;
    }
    result[i][i] = 1;
  }
}

// Multiply two square matrices
void multiply(Matrix &result, const Matrix &a, const Matrix &b) {
  zero(result);

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      for (int k = 0; k < N; k++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

constexpr Matrix identity() {
  Matrix result{};
  for (int i = 0; i < N; i++) {
    result[i][i] = 1;
  }
  return result;
}

void multiply2(const MatrixView a, const MatrixView b, MatrixView out) {
  out.raw() = {};
  for (int i = 0; i < N; i++) {
    for (int k = 0; k < N; k++) {
      for (int j = 0; j < N; j++) {
        out[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

// Compute integer power of a given square matrix
Matrix power(const Matrix &input, const uint32_t k) {
  if(k == 0) [[unlikely]]{
    return identity();
  }

  if(k == 1) [[unlikely]]{
    return input;
  }
  // store everything on stack, no dynamic allocations
  // products
  auto productCurrent = identity();
  Matrix productTmp;
  MatrixView productCurrentView{productCurrent};
  MatrixView productTmpView{productTmp};

  // elements
  auto elementCurrent = input;
  Matrix elementTmp;
  MatrixView elementCurrentView{elementCurrent};
  MatrixView elementTmpView{elementTmp};

  for (auto i = k; i != 1; i >>= 2) {
    // `k` is `2021`, it has only 2 zero bits, thus, we can use `likely`
    const auto isEven = (i & 1);
    if (isEven) [[likely]] {
      multiply2(productCurrentView, elementCurrentView, productTmpView);
      // using views allows us to efficiently swap matrices
      swap(productTmpView, productCurrentView);
    }

    multiply2(elementCurrentView, elementCurrentView, elementTmpView);
    swap(elementTmpView, elementCurrentView);
  }

  multiply2(productCurrentView, elementCurrentView, productTmpView);
  swap(productTmpView, productCurrentView);

  return productCurrent;
}
