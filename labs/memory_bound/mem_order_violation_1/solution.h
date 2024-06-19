#include <cstdint>
#include <memory>
#include <array>
#include <string>

// https://en.wikipedia.org/wiki/Grayscale
struct GrayscaleImage {
  std::unique_ptr<uint8_t[]> data;
  size_t size = 0;
  int width = 0;
  int height = 0;

  bool load(const std::string &filename, const int maxSize);
  void save(const std::string &filename);
};

// Binary (aka monochrome) image
// https://en.wikipedia.org/wiki/Binary_image
// We will represent binary image in the same way as grayscale.
using BinaryImage = GrayscaleImage;

constexpr int kMaxImageDimension = 32 * 1024;

std::array<uint32_t, 256> computeHistogram(const GrayscaleImage& image);
std::array<uint32_t, 256> computeHistogram_solution(const GrayscaleImage& image);
int calcOtsuThreshold(const std::array<uint32_t, 256>& hist, int totalPixels);
void applyOtsuThreshold(GrayscaleImage& image);
