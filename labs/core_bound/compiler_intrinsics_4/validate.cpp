#include "const.h"
#include "data_paths.h"
#include "picture.h"
#include "solution.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

namespace {

constexpr auto kImageWidth = 640;
constexpr auto kImageHeight = 360;

std::vector<short> reference_mandelbrot(int image_width, int image_height) {
  const auto data_width = image_width + 2;
  const auto data_height = image_height + 2;
  const auto diameter_y = kDiameterX / image_width * image_height;
  const auto min_x = kCenterX - kDiameterX / 2;
  const auto max_x = kCenterX + kDiameterX / 2;
  const auto min_y = kCenterY - diameter_y / 2;
  const auto max_y = kCenterY + diameter_y / 2;
  std::vector<short> result(data_width * data_height);
  auto result_idx = 0;
  for (auto py = 0; py < data_height; ++py) {
    for (auto px = 0; px < data_width; ++px) {
      const auto c_x = std::lerp(min_x, max_x, 1.0 * px / data_width);
      const auto c_y = std::lerp(min_y, max_y, 1.0 * py / data_height);
      auto z_x = 0.0;
      auto z_y = 0.0;
      auto iter_cnt = 0;
      for (; iter_cnt < kMaxIterations; ++iter_cnt) {
        const auto z_xx = z_x * z_x;
        const auto z_yy = z_y * z_y;
        if (z_xx + z_yy > kSquareBound) {
          break;
        }
        const auto z_xy = z_x * z_y;
        z_x = z_xx - z_yy + c_x;
        z_y = z_xy + z_xy + c_y;
      }
      result[result_idx++] = iter_cnt;
    }
  }
  return result;
}

} // namespace

int main() {
  const auto ref_data = reference_mandelbrot(kImageWidth, kImageHeight);
  const auto data = mandelbrot(kImageWidth, kImageHeight);

  if (data.size() != ref_data.size()) {
    std::cerr << "Result has invalid size. Expected size: " << ref_data.size()
              << " received: " << data.size() << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Saving image to " << validation_image_path << std::endl;
  std::ofstream(validation_image_path, std::ios::binary) << generate_ppm_image(data, kImageWidth, kImageHeight);
  std::cout << "Saving reference image to " << reference_image_path << std::endl;
  std::ofstream(reference_image_path, std::ios::binary) << generate_ppm_image(ref_data, kImageWidth, kImageHeight);

  const auto cmp_result = std::mismatch(data.begin(), data.end(), ref_data.begin());
  if (cmp_result.first != data.end()) {
    std::cerr << "Validation Failed at position: "
              << std::distance(data.begin(), cmp_result.first)
              << ". Expected: " << *cmp_result.second
              << " received: " << *cmp_result.first
              << "." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Validation Successful" << std::endl;
  return EXIT_SUCCESS;
}
