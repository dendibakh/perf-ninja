
#include "solution.h"
#include <algorithm>
#include <fstream>
#include <ios>

// Applies Gaussian blur in independent vertical lines
static void filterVertically(uint8_t *output, const uint8_t *input,
                             const int width, const int height,
                             const int *kernel, const int radius,
                             const int shift) {
  const int rounding = 1 << (shift - 1);

  for (int c = 0; c < width; c++) {
    // Top part of line, partial kernel
    for (int r = 0; r < std::min(radius, height); r++) {
      // Accumulation
      int dot = 0;
      int sum = 0;
      auto p = &kernel[radius - r];
      for (int y = 0; y <= std::min(r + radius, height - 1); y++) {
        int weight = *p++;
        dot += input[y * width + c] * weight;
        sum += weight;
      }

      // Normalization
      int value = static_cast<int>(dot / static_cast<float>(sum) + 0.5f);
      output[r * width + c] = static_cast<uint8_t>(value);
    }

    // Middle part of computations with full kernel
    for (int r = radius; r < height - radius; r++) {
      // Accumulation
      int dot = 0;
      for (int i = 0; i < radius + 1 + radius; i++) {
        dot += input[(r - radius + i) * width + c] * kernel[i];
      }

      // Fast shift instead of division
      int value = (dot + rounding) >> shift;
      output[r * width + c] = static_cast<uint8_t>(value);
    }

    // Bottom part of line, partial kernel
    for (int r = std::max(radius, height - radius); r < height; r++) {
      // Accumulation
      int dot = 0;
      int sum = 0;
      auto p = kernel;
      for (int y = r - radius; y < height; y++) {
        int weight = *p++;
        dot += input[y * width + c] * weight;
        sum += weight;
      }

      // Normalization
      int value = static_cast<int>(dot / static_cast<float>(sum) + 0.5f);
      output[r * width + c] = static_cast<uint8_t>(value);
    }
  }
}

// Applies Gaussian blur in independent horizontal lines
static void filterHorizontally(uint8_t *output, const uint8_t *input,
                               const int width, const int height,
                               const int *kernel, const int radius,
                               const int shift) {
  const int rounding = 1 << (shift - 1);

  for (int r = 0; r < height; r++) {
    // Left part of line, partial kernel
    for (int c = 0; c < std::min(radius, width); c++) {
      // Accumulation
      int dot = 0;
      int sum = 0;
      auto p = &kernel[radius - c];
      for (int x = 0; x <= std::min(c + radius, width - 1); x++) {
        int weight = *p++;
        dot += input[r * width + x] * weight;
        sum += weight;
      }

      // Normalization
      int value = static_cast<int>(dot / static_cast<float>(sum) + 0.5f);
      output[r * width + c] = static_cast<uint8_t>(value);
    }

    // Middle part of computations with full kernel
    for (int c = radius; c < width - radius; c++) {
      // Accumulation
      int dot = 0;
      for (int i = 0; i < radius + 1 + radius; i++) {
        dot += input[r * width + c - radius + i] * kernel[i];
      }

      // Fast shift instead of division
      int value = (dot + rounding) >> shift;
      output[r * width + c] = static_cast<uint8_t>(value);
    }

    // Right part of line, partial kernel
    for (int c = std::max(radius, width - radius); c < width; c++) {
      // Accumulation
      int dot = 0;
      int sum = 0;
      auto p = kernel;
      for (int x = c - radius; x < width; x++) {
        int weight = *p++;
        dot += input[r * width + x] * weight;
        sum += weight;
      }

      // Normalization
      int value = static_cast<int>(dot / static_cast<float>(sum) + 0.5f);
      output[r * width + c] = static_cast<uint8_t>(value);
    }
  }
}

// Applies Gaussian blur to a grayscale image
void blur(uint8_t *output, const uint8_t *input, const int width,
          const int height, uint8_t *temp) {
  // Integer Gaussian blur with kernel size 5
  // https://en.wikipedia.org/wiki/Kernel_(image_processing)
  constexpr int radius = 2;
  constexpr int kernel[radius + 1 + radius] = {1, 4, 6, 4, 1};
  // An alternative to division by power of two = sum(kernel)
  constexpr int shift = 4;

  // A pair of 1-dimensional passes to achieve 2-dimensional transform
  filterVertically(temp, input, width, height, kernel, radius, shift);
  filterHorizontally(output, temp, width, height, kernel, radius, shift);
}

// Loads grayscale image. Format is
// https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html Function doesn't
// support comments.
bool Grayscale::load(const std::string &filename, const int maxSize) {
  data.reset();

  std::ifstream input(filename.data(),
                      std::ios_base::in | std::ios_base::binary);
  if (input.is_open()) {
    std::string line;
    input >> line;
    if (line == "P5") {
      int amplitude;
      input >> width >> height >> amplitude;

      char c;
      input.unsetf(std::ios_base::skipws);
      input >> c;

      if ((width > 0) && (width <= maxSize) && (height > 0) &&
          (height <= maxSize) && (amplitude >= 0) && (amplitude <= 255) &&
          (c == '\n')) {
        size = static_cast<size_t>(width) * static_cast<size_t>(height);
        data.reset(new uint8_t[size]);
        if (data) {
          input.read(reinterpret_cast<char *>(data.get()), size);
          if (input.fail()) {
            data.reset();
          }
        }
      }
    }

    input.close();
  }

  return !!data;
}

// Saves grayscale image. Format is
// https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html
void Grayscale::save(const std::string &filename) {
  std::ofstream output(filename.data(),
                       std::ios_base::out | std::ios_base::binary);
  if (output.is_open()) {
    output << "P5" << std::endl
           << width << ' ' << height << std::endl
           << "255" << std::endl;
    if (data) {
      output.write(reinterpret_cast<const char *>(data.get()), size);
    }
    output.close();
  }
}
