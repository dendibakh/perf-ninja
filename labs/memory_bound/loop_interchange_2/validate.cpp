
#include "solution.h"
#include <cstring>
#include <iostream>

int main(int argc, char **argv) {
  constexpr int mandatoryArgumentsCount = 2;
  if (argc != 1 + mandatoryArgumentsCount) {
    std::cerr << "Usage: input.pgm output-golden.pgm" << std::endl;
    return 1;
  }
  const std::string input = argv[1];
  const std::string outputGolden = argv[2];

  Grayscale image;
  if (image.load(input, kMaxImageDimension)) {
    Grayscale image2;
    if (image2.load(outputGolden, kMaxImageDimension)) {
      // Copied & pasted
      {
        decltype(Grayscale::data) result(new uint8_t[image.size]);
        decltype(Grayscale::data) temp(new uint8_t[image.size]);
        if (!result || !temp) {
          std::cerr << "Validation Failed. Out of memory" << std::endl;
          return 1;
        }

        blur(result.get(), image.data.get(), image.width, image.height,
             temp.get());
        std::swap(image.data, result);
      }

      if ((image.width != image2.width) || (image.height != image2.height)) {
        std::cerr << "Validation Failed. Wrong image dimensions " << image.width
                  << 'x' << image.height << std::endl;
        return 1;
      }

      auto p1 = image.data.get();
      auto p2 = image2.data.get();
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
  }

  std::cerr << "Validation Failed. An IO problem" << std::endl;
  return 1;
}
