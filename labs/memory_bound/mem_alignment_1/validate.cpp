
#include "solution.h"
#include <cmath>
#include <iostream>

static void original_matmul(float* RESTRICT A, 
                            float* RESTRICT B,
                            float* RESTRICT C, int N) {
  for (int i = 0; i < N; ++i)
    for (int k = 0; k < N; ++k)
      for (int j = 0; j < N; ++j)
        C[i * N + j] += A[i * N + k] * B[k * N + j];
}

// Both matrices have N x N elements, but matrix A can have padding, 
// so matrix A can have dimensions N x K, even though only N x N are used.
static bool equals(const Matrix &a, const Matrix &b, int N, int K) {
  constexpr int maxErrors = 10;
  const float epsilon = std::sqrt(std::numeric_limits<float>::epsilon());

  int errors = 0;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      float va = a[i * K + j];
      float vb = b[i * N + j];
      float error = std::abs(va - vb);
      if (error >= epsilon) {
        std::cerr << "Result[" << i << ", " << j << "] = " << va
                  << ". Expected[" << i << ", " << j << "] = " << vb
                  << std::endl;
        if (++errors >= maxErrors)
          return false;
      }
    }
  }
  return 0 == errors;
}

int smokeTest() {
  const int N = 5;
    Matrix a1 = {
         0.12f, -0.34f,  0.56f,  0.78f, -0.90f,
         0.11f,  0.22f, -0.33f,  0.44f,  0.55f,
        -0.66f,  0.77f, -0.88f,  0.99f,  0.10f,
         0.21f,  0.32f,  0.43f,  0.54f,  0.65f,
        -0.12f, -0.23f, -0.34f, -0.45f,  0.56f
    };

    Matrix b1 = {
         0.45f, -0.55f,  0.65f, -0.75f,  0.85f,
        -0.15f,  0.25f, -0.35f,  0.45f, -0.55f,
         0.85f, -0.75f,  0.65f, -0.55f,  0.45f,
        -0.25f,  0.35f, -0.45f,  0.55f, -0.65f,
         0.55f, -0.45f,  0.35f, -0.25f,  0.15f
    };

    Matrix expected = {
        -0.109f,  0.107f, -0.105f,  0.103f, -0.101f,
        -0.0715f, 0.1485f,-0.2255f, 0.3025f,-0.3795f,
        -1.353f,  1.517f, -1.681f,  1.845f, -2.009f,
         0.6345f,-0.4615f, 0.2885f,-0.1155f,-0.0575f,
         0.112f, -0.146f,  0.18f,  -0.214f,  0.248f
    };

  Matrix c1; c1.resize(N * N);  initZero (c1, N, N);
  original_matmul    (a1.data(), b1.data(), c1.data(), N);
  if (!equals(c1, expected, N, N))
    return 1;

  const int K = n_columns(N);
  Matrix a2; a2.resize(N * K);  copyFromMatrix(a1, a2, N, K);
  Matrix b2; b2.resize(N * K);  copyFromMatrix(b1, b2, N, K);
  Matrix c2; c2.resize(N * K);  initZero      (c2, N, K);

  interchanged_matmul(a2.data(), b2.data(), c2.data(), N, K);
  if (!equals(c2, expected, N, K))
    return 1;

  initZero      (c2, N, K);
  blocked_matmul(a2.data(), b2.data(), c2.data(), N, K);
  if (!equals(c2, expected, N, K))
    return 1;

  return 0;
}

int testSize(int N) {
  const int K = n_columns(N);

  Matrix a1; a1.resize(N * N);  initRandom    (a1, N, N);
  Matrix b1; b1.resize(N * N);  initRandom    (b1, N, N);
  Matrix c1; c1.resize(N * N);  initZero      (c1, N, N);
  Matrix a2; a2.resize(N * K);  copyFromMatrix(a1, a2, N, K);
  Matrix b2; b2.resize(N * K);  copyFromMatrix(b1, b2, N, K);
  Matrix c2; c2.resize(N * K);  initZero      (c2, N, K);

  original_matmul    (a1.data(), b1.data(), c1.data(), N);
  interchanged_matmul(a2.data(), b2.data(), c2.data(), N, K);
  if (!equals(c2, c1, N, K)) {
    std::cerr << "Validation Failed: interchanged_matmul " << N << std::endl;
    return 1;
  }

  initZero      (c2, N, K);
  blocked_matmul(a2.data(), b2.data(), c2.data(), N, K);
  if (!equals(c2, c1, N, K)) {
    std::cerr << "Validation Failed: blocked_matmul " << N << std::endl;
    return 1;
  }

  return 0;
}

int main() {
  if (smokeTest()) {
    std::cerr << "Smoke test failed: " << std::endl;
    return 1;
  }
  if (testSize(1))  return 1;
  if (testSize(10)) return 1;
  if (testSize(63)) return 1;
  if (testSize(64)) return 1;
  if (testSize(65)) return 1;
  if (testSize(127)) return 1;
  if (testSize(128)) return 1;
  if (testSize(129)) return 1;
  if (testSize(255)) return 1;
  if (testSize(256)) return 1;
  if (testSize(257)) return 1;
  if (testSize(511)) return 1;
  if (testSize(512)) return 1;
  if (testSize(513)) return 1;  
  if (testSize(1023)) return 1;
  if (testSize(1024)) return 1;
  if (testSize(1025)) return 1;    
  std::cout << "Validation Successful" << std::endl;  
}
