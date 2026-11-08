
#include "solution.h"
#include <algorithm>
#include <fstream>
#include <stdint.h>
#include <cmath>
#include <ios>
// #include <immintrin.h>

// ******************************************
// ONLY THE FOLLOWING FUNCTION IS BENCHMARKED
// Compute the histogram of image pixels

// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bird/0           77.4 us         77.4 us         8937
// coins/1          35.2 us         35.2 us        19990
// pepper/2         30.6 us         30.6 us        22418
// pixabay/3        14.7 ms         14.7 ms           49

std::array<uint32_t, 256> computeHistogram_2(const GrayscaleImage& image) {
  std::array<uint32_t, 256> hist;
  hist.fill(0);

  for (int i = 0; i < image.width * image.height; ++i){
    hist[image.data[i]]++;
  }
  return hist;
}

// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bird/0           56.2 us         56.2 us        12459
// coins/1          27.4 us         27.4 us        25535
// pepper/2         24.8 us         24.8 us        28618
// pixabay/3        11.5 ms         11.5 ms           61

// .L4:
//         movdqa  1024(%rsp,%rax), %xmm0
//         paddd   (%rsp,%rax), %xmm0
//         paddd   3072(%rsp,%rax), %xmm0
//         paddd   2048(%rsp,%rax), %xmm0
//         movups  %xmm0, (%rbx,%rax)

std::array<uint32_t, 256> computeHistogram_simple(const GrayscaleImage& image) {
  std::array<uint32_t, 256> hist_1;
  std::array<uint32_t, 256> hist_2;
  std::array<uint32_t, 256> hist_3;
  std::array<uint32_t, 256> hist_4;

  hist_1.fill(0);
  hist_2.fill(0);
  hist_3.fill(0);
  hist_4.fill(0);

  int chunks = (image.width * image.height)/4;
  int main_iter_count = chunks * 4;

  for (int i = 0; i < main_iter_count; i+=4){
    hist_1[image.data[i]]++;
    hist_2[image.data[i+1]]++;
    hist_3[image.data[i+2]]++;
    hist_4[image.data[i+3]]++;
  }

  std::array<uint32_t, 256> hist_final;
  hist_final.fill(0);

  for (int i = 0; i < 256; ++i) {
    hist_final[i] =
        hist_1[i] + hist_2[i] +
        hist_3[i] + hist_4[i];
  }

  for (int i = main_iter_count; i < image.width * image.height; ++i){
    hist_final[image.data[i]]++;
  }

  return hist_final;
}

std::array<uint32_t, 256> computeHistogram(const GrayscaleImage& image) {
  std::array<uint32_t, 256> hist_1;
  std::array<uint32_t, 256> hist_2;
  std::array<uint32_t, 256> hist_3;
  std::array<uint32_t, 256> hist_4;
  std::array<uint32_t, 256> hist_5;
  std::array<uint32_t, 256> hist_6;

  hist_1.fill(0);
  hist_2.fill(0);
  hist_3.fill(0);
  hist_4.fill(0);
  hist_5.fill(0);
  hist_6.fill(0);

  int chunks = (image.width * image.height)/6;
  int main_iter_count = chunks * 6;

  for (int i = 0; i < main_iter_count; i+=6){
    hist_1[image.data[i]]++;
    hist_2[image.data[i+1]]++;
    hist_3[image.data[i+2]]++;
    hist_4[image.data[i+3]]++;
    hist_5[image.data[i+4]]++;
    hist_6[image.data[i+5]]++;
  }

  std::array<uint32_t, 256> hist_final;
  hist_final.fill(0);

  for (int i = 0; i < 256; ++i) {
    hist_final[i] =
        hist_1[i] + hist_2[i] +
        hist_3[i] + hist_4[i] + 
        hist_5[i] + hist_6[i];
  }

  for (int i = main_iter_count; i < image.width * image.height; ++i){
    hist_final[image.data[i]]++;
  }

  return hist_final;
}

// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bird/0           56.0 us         56.0 us        12058
// coins/1          27.3 us         27.3 us        25935
// pepper/2         24.4 us         24.4 us        29380
// pixabay/3        11.7 ms         11.7 ms           61
// std::array<uint32_t, 256> computeHistogram_SIMD(const GrayscaleImage& image) {
//   std::array<uint32_t, 256> hist_1;
//   std::array<uint32_t, 256> hist_2;
//   std::array<uint32_t, 256> hist_3;
//   std::array<uint32_t, 256> hist_4;

//   hist_1.fill(0);
//   hist_2.fill(0);
//   hist_3.fill(0);
//   hist_4.fill(0);

//   int chunks = (image.width * image.height)/4;
//   int main_iter_count = chunks * 4;

//   for (int i = 0; i < main_iter_count; i+=4){
//     hist_1[image.data[i]]++;
//     hist_2[image.data[i+1]]++;
//     hist_3[image.data[i+2]]++;
//     hist_4[image.data[i+3]]++;
//   }

//   std::array<uint32_t, 256> hist_final;
//   hist_final.fill(0);

//   for (int i = 0; i < 256; i += 8) {
//     __m256i h1 = _mm256_loadu_si256(
//         reinterpret_cast<const __m256i*>(&hist_1[i]));

//     __m256i h2 = _mm256_loadu_si256(
//         reinterpret_cast<const __m256i*>(&hist_2[i]));

//     __m256i h3 = _mm256_loadu_si256(
//         reinterpret_cast<const __m256i*>(&hist_3[i]));

//     __m256i h4 = _mm256_loadu_si256(
//         reinterpret_cast<const __m256i*>(&hist_4[i]));

//     __m256i sum = _mm256_add_epi32(h1, h2);
//     sum = _mm256_add_epi32(sum, h3);
//     sum = _mm256_add_epi32(sum, h4);

//     _mm256_storeu_si256(
//         reinterpret_cast<__m256i*>(&hist_final[i]), sum);
// }

//   for (int i = main_iter_count; i < image.width * image.height; ++i){
//     hist_final[image.data[i]]++;
//   }

//   return hist_final;
// }

// ******************************************

// Calculate Otsu's Threshold
int calcOtsuThreshold(const std::array<uint32_t, 256>& hist, int totalPixels) {
  // normalize histogram
  std::array<double, 256> normHist;
  for (int i = 0; i < 256; ++i)
    normHist[i] = (double)hist[i] / totalPixels;

  double maxVariance = 0;
  int optimalThreshold = 0;

  // Find the optimal threshold
  for (int t = 0; t < 256; ++t) {
    double weight1 = 0, weight2 = 0, mean1 = 0, mean2 = 0;

    for (int i = 0; i <= t; ++i) {
      weight1 += normHist[i];
      mean1 += i * normHist[i];
    }

    for (int i = t + 1; i < 256; ++i) {
      weight2 += normHist[i];
      mean2 += i * normHist[i];
    }

    if (weight1 == 0 || weight2 == 0) continue;

    mean1 /= weight1;
    mean2 /= weight2;

    double variance = weight1 * weight2 * std::pow(mean1 - mean2, 2);

    if (variance > maxVariance) {
      maxVariance = variance;
      optimalThreshold = t;
    }
  }

  return optimalThreshold;
}

// Function to apply the threshold to create a binary image
void applyOtsuThreshold(GrayscaleImage& image) {
  // Compute the histogram
  std::array<uint32_t, 256> hist = computeHistogram(image);
  auto totalPixels = image.height * image.width;
  int threshold = calcOtsuThreshold(hist, totalPixels);
  // Apply Otsu's thresholding
  for (int i = 0; i < totalPixels; ++i)
    image.data[i] = (image.data[i] >= threshold) ? 255 : 0;
}

// Loads GrayscaleImage image. Format is
// https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html 
bool GrayscaleImage::load(const std::string &filename, const int maxSize) {
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
      if (c == '\r')
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

// Saves GrayscaleImage image. Format is
// https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html
void GrayscaleImage::save(const std::string &filename) {
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
