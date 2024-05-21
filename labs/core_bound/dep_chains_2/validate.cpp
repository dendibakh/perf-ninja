#include "solution.hpp"
#include <iostream>
#include <limits>
#include <cmath>
#include <random>

static bool equals(const std::vector<Particle> &p1, const std::vector<Particle> &p2) {
  constexpr int maxErrors = 10;
  const float epsilon = std::sqrt(std::numeric_limits<float>::epsilon());
  int errors = 0;

  for (int i = 0; i < p1.size(); i++) {
    float x1 = p1[i].x;
    float x2 = p2[i].x;
    float y1 = p1[i].y;
    float y2 = p2[i].y;
    float xerror = std::abs(x1 - x2);
    float yerror = std::abs(y1 - y2);
    if (xerror >= epsilon || yerror >= epsilon) {
      std::cerr << "Result: p1[" << i << "] = {" << x1 << "," << y1 << "}"
                << ". Expected : p1[" << i << "] = {" << x2 << "," << y2 << "}"
                << std::endl;
      if (++errors >= maxErrors)
        return false;
      }
  }
  return 0 == errors;
}

// For validation we use a deterministic random number generator
// that uses a global state and thus always generates the same sequence of numbers
struct rngForValidation {
  rngForValidation (uint32_t seed) {(void)seed;}
public:
  static uint32_t val;
  uint32_t gen() {
    return val++;
  }
};

uint32_t rngForValidation::val = 0;

void randomParticleMotionOriginal(std::vector<Particle> &particles, uint32_t seed) {
  rngForValidation rng(seed);  
  for (int i = 0; i < STEPS; i++)
    for (auto &p : particles) {
      uint32_t angle = rng.gen();
      float angle_rad = angle * DEGREE_TO_RADIAN;
      p.x += cosine(angle_rad) * p.velocity;
      p.y += sine(angle_rad) * p.velocity;
    }
}

int main() {
  // Init benchmark data
  auto particles = initParticles();
  auto particlesCopy = particles;

  std::random_device r;
  std::mt19937_64 random_engine(r());
  std::uniform_int_distribution<uint32_t> distrib(0, std::numeric_limits<uint32_t>::max());

  auto seed = distrib(random_engine);
  randomParticleMotionOriginal(particlesCopy, seed);
  rngForValidation::val = 0;
  randomParticleMotion<rngForValidation>(particles, seed);
  
  if (!equals(particlesCopy, particles)) {
    std::cerr << "Validation Failed." << "\n";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
