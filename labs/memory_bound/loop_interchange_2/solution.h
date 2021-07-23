
#include <cstdint>
#include <memory>
#include <string>

// Applies Gaussian blur to a grayscale image
void blur(uint8_t *output, const uint8_t *input, const int width,
          const int height, uint8_t *temp);

struct Grayscale {
  std::unique_ptr<uint8_t[]> data;
  size_t size = 0;
  int width = 0, height = 0;

  bool load(const std::string &filename, const int maxSize);
  void save(const std::string &filename);
};

constexpr int kMaxImageDimension = 32 * 1024;
