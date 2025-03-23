
#include "solution.h"
#include <algorithm>
#include <fstream>
#include <stdint.h>
#include <cmath>
#include <ios>

// ******************************************
// ONLY THE FOLLOWING FUNCTION IS BENCHMARKED
// Compute the histogram of image pixels
std::array<uint32_t, 256> computeHistogram(const GrayscaleImage& image) {
  /*
    problem here is that, if there is consequence of the same color
    in the picture, there would be data dependency. to break it
    we can use saveral hists. so that if we have 4 sequent elements
    this 4 elements would pretend to different data addresses.
    so load and stores to these hists can be reordered. 
  */
  
#define SOLUTION
/*
TLDR: Best number of hists is 16. That's seems enough to break dependencies
and let pipeline reorder load-stores. That's importatnt that that's really depends on
sepecific underlying data. As if data is diverse processor will be able to reorder
load-stores without our efforts.

    No optimization:
      bird/0           52.3 us         52.3 us        13406
      coins/1          25.3 us         25.2 us        27327
      pepper/2         18.3 us         18.2 us        37453
      pixabay/3        8.03 ms         7.97 ms           87

    32 hists:
      bird/0           37.9 us         37.9 us        18281
      coins/1          18.7 us         18.7 us        37706
      pepper/2         16.4 us         16.4 us        42487
      pixabay/3        7.60 ms         7.55 ms           93

    16 hists:
      bird/0           37.1 us         37.0 us        18564
      coins/1          18.4 us         18.4 us        37074
      pepper/2         16.1 us         16.0 us        43733
      pixabay/3        7.51 ms         7.49 ms           93

    8 hists:
      bird/0           37.4 us         37.4 us        18601
      coins/1          18.8 us         18.8 us        36659
      pepper/2         16.0 us         16.0 us        43101
      pixabay/3        7.66 ms         7.66 ms           88

    4 hists:
      bird/0           38.3 us         38.3 us        18233
      coins/1          19.1 us         19.1 us        36303
      pepper/2         16.4 us         16.4 us        42973
      pixabay/3        7.75 ms         7.75 ms           85

    2 hists:
      bird/0           46.1 us         46.1 us        15048
      coins/1          22.0 us         22.0 us        32083
      pepper/2         20.5 us         20.5 us        33829
      pixabay/3        8.16 ms         8.16 ms           85

    1 hists:
      bird/0           47.9 us         47.9 us        14314
      coins/1          24.5 us         24.5 us        28875
      pepper/2         17.8 us         17.7 us        39422
      pixabay/3        8.10 ms         8.10 ms           86
*/

#ifdef SOLUTION
  constexpr int HistNum = 16;
  using hist_t = std::array<uint32_t, 256>;
  std::array<hist_t, HistNum> hists;
  hist_t hist;

  for (auto& hist : hists) {
    hist.fill(0);
  }

  int tail_size = (image.width * image.height) % HistNum;
  int body_size = (image.width * image.height / HistNum) * HistNum;
  for (int i = 0; i < body_size; i += HistNum) {
    for (int hist_idx = 0; hist_idx < HistNum; hist_idx++) {
      hists[hist_idx][image.data[i + hist_idx]]++;
    }
  }

  for (int i = 0; i < tail_size; i++) {
    hists[0][image.data[body_size + i]]++;
  }

  for (int i = 0; i < hist.size(); i++) {
    int total = 0;
    for (int hist_idx = 0; hist_idx < HistNum; hist_idx++) {
      total += hists[hist_idx][i];
    }
    hist[i] = total;
  }
#else
  std::array<uint32_t, 256> hist;
  hist.fill(0);
  for (int i = 0; i < image.width * image.height; ++i)
    hist[image.data[i]]++;
#endif
    return hist;
}
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
