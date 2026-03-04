#include "solution.h"
#include <algorithm>
#include <fstream>
#include <ios>
#include <cstdint>

// Cache-friendly tiled transpose: out has shape (width x height) if in is (height x width)
// i.e. out[x * height + y] = in[y * width + x]
static inline void transpose_u8_tiled(uint8_t* out, const uint8_t* in, int width, int height) {
  constexpr int TILE = 32;

  for (int y0 = 0; y0 < height; y0 += TILE) {
    const int yMax = std::min(y0 + TILE, height);
    for (int x0 = 0; x0 < width; x0 += TILE) {
      const int xMax = std::min(x0 + TILE, width);

      for (int y = y0; y < yMax; ++y) {
        const uint8_t* row = in + y * width;
        for (int x = x0; x < xMax; ++x) {
          out[x * height + y] = row[x];
        }
      }
    }
  }
}

// Applies Gaussian blur in independent horizontal lines (your existing implementation)
static void filterHorizontally(uint8_t *output, const uint8_t *input,
                               const int width, const int height,
                               const int *kernel, const int radius,
                               const int shift) {
  const int rounding = 1 << (shift - 1);

  for (int r = 0; r < height; r++) {
    for (int c = 0; c < std::min(radius, width); c++) {
      int dot = 0;
      int sum = 0;
      auto p = &kernel[radius - c];
      for (int x = 0; x <= std::min(c + radius, width - 1); x++) {
        int weight = *p++;
        dot += input[r * width + x] * weight;
        sum += weight;
      }
      int value = static_cast<int>(dot / static_cast<float>(sum) + 0.5f);
      output[r * width + c] = static_cast<uint8_t>(value);
    }

    for (int c = radius; c < width - radius; c++) {
      int dot = 0;
      for (int i = 0; i < radius + 1 + radius; i++) {
        dot += input[r * width + c - radius + i] * kernel[i];
      }
      int value = (dot + rounding) >> shift;
      output[r * width + c] = static_cast<uint8_t>(value);
    }

    for (int c = std::max(radius, width - radius); c < width; c++) {
      int dot = 0;
      int sum = 0;
      auto p = kernel;
      for (int x = c - radius; x < width; x++) {
        int weight = *p++;
        dot += input[r * width + x] * weight;
        sum += weight;
      }
      int value = static_cast<int>(dot / static_cast<float>(sum) + 0.5f);
      output[r * width + c] = static_cast<uint8_t>(value);
    }
  }
}

// Vertical blur via transpose + horizontal blur + transpose back.
// Uses:
//   temp: size width*height (scratch + final vertical result)
//   output: used as intermediate during the vertical stage (also size width*height)
static inline void filterVerticallyViaTranspose(uint8_t* temp,
                                                const uint8_t* input,
                                                uint8_t* output,
                                                int width, int height,
                                                const int* kernel, int radius,
                                                int shift) {
  // 1) transpose input (w x h) -> temp (h x w)
  transpose_u8_tiled(temp, input, width, height);

  // 2) horizontal blur on transposed image:
  //    dims are (width' = height, height' = width)
  //    write into output (still transposed layout)
  filterHorizontally(output, temp, /*width=*/height, /*height=*/width, kernel, radius, shift);

  // 3) transpose back output (h x w) -> temp (w x h)
  transpose_u8_tiled(temp, output, /*width=*/height, /*height=*/width);
}

// Applies Gaussian blur to a grayscale image
void blur(uint8_t *output, const uint8_t *input, const int width,
          const int height, uint8_t *temp) {
  constexpr int radius = 2;
  constexpr int kernel[radius + 1 + radius] = {1, 4, 6, 4, 1};
  constexpr int shift = 4;

  // Vertical pass optimized: temp becomes the vertical-blurred image in original layout.
  // output is used as intermediate scratch during the vertical stage.
  filterVerticallyViaTranspose(temp, input, output, width, height, kernel, radius, shift);

  // Final horizontal pass
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
