
#include "solution.h"
#include <cmath>
#include <limits>
#include <random>

void init(Matrix &matrix) {
  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(-0.95f, 0.95f);

  for (int i = 0; i < N; i++) {
    float sum = 0;
    for (int j = 0; j < N; j++) {
      float value = distribution(generator);
      sum += value * value;
      matrix[i][j] = value;
    }

    // Normalize rows
    if (sum >= std::numeric_limits<float>::min()) {
      float scale = 1.0f / std::sqrt(sum);
      for (int j = 0; j < N; j++) {
        matrix[i][j] *= scale;
      }
    }
  }
}
