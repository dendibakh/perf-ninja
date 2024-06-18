
#include "solution.h"
#include <cstring>
#include <iostream>

static std::array<uint32_t, 256> computeHistogram_original(const GrayscaleImage& image) {
  std::array<uint32_t, 256> hist;
  hist.fill(0);
  for (int i = 0; i < image.width * image.height; ++i)
    hist[image.data[i]]++;
  return hist;
}

int main(int argc, char **argv) {
  constexpr int mandatoryArgumentsCount = 2;
  if (argc != 1 + mandatoryArgumentsCount) {
    std::cerr << "Usage: input.pgm output-golden.pgm" << std::endl;
    return 1;
  }
  const std::string input = argv[1];
  const std::string outputGolden = argv[2];

  GrayscaleImage image;
  if (!image.load(input, kMaxImageDimension)) {
    std::cerr << "Cannot load input image. Validation Failed." << std::endl;
    return 1;
  }

  // smoke test
  auto h1 = computeHistogram(image);
  auto h2 = computeHistogram_original(image);
  if (h1 != h2) {
    std::cerr << "Validation Failed. Smoke test fails" << std::endl;
    int downcount = 10;
    for (size_t i = 0; i < h1.size(); i++) {
      if (h1[i] != h2[i]) {
        std::cerr << "Result[" << i << "] = " << static_cast<int>(h1[i])
                  << ". Expected[" << i << "] = " << static_cast<int>(h2[i])
                  << std::endl;
        if (--downcount <= 0)
          break;
      }
    }
  }

  // testing the whole algorithm on a real image
  BinaryImage goldenImage;
  if (!goldenImage.load(outputGolden, kMaxImageDimension)) {
    std::cerr << "Cannot load golden image. Validation Failed." << std::endl;
    return 1;
  }
  applyOtsuThreshold(image);

  if ((image.width != goldenImage.width) || (image.height != goldenImage.height)) {
    std::cerr << "Validation Failed. Wrong image dimensions " << image.width
              << 'x' << image.height << std::endl;
    return 1;
  }

  auto p1 = image.data.get();
  auto p2 = goldenImage.data.get();
  if (std::memcmp(p1, p2, image.size) == 0) {
    std::cout << "Validation Successful" << std::endl;
    return 0;
  }

  std::cerr << "Validation Failed. Distinct images" << std::endl;
  int downcount = 10;
  for (size_t i = 0; i < image.size; i++) {
    if (p1[i] != p2[i]) {
      std::cerr << "Result[" << i << "] = " << static_cast<int>(p1[i])
                << ". Expected[" << i << "] = " << static_cast<int>(p2[i])
                << std::endl;
      if (--downcount <= 0)
        break;
    }
  }
  return 1;  
}
