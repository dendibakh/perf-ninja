#include <vector>
#include <iostream>
#include <cstdint>
#include <array>

// The number of motion simulation steps.
constexpr uint32_t STEPS = 10000;
// The number of paticles to simulate.
constexpr uint32_t PARTICLES = 1000;

struct Particle
{
  float x;
  float y;
  float velocity;
};

// Initialize the particles with random coordinates and velocities.
std::vector<Particle> initParticles();

// Medium-quality random number generator.
// https://www.javamex.com/tutorials/random_numbers/xorshift.shtml
struct XorShift32
{
  uint32_t val;
  XorShift32(uint32_t seed) : val(seed) {}

public:
  uint32_t gen()
  {
    val ^= (val << 13);
    val ^= (val >> 17);
    val ^= (val << 5);
    return val;
  }
};

constexpr double PI_D = 3.141592653589793238463;
constexpr float PI_F = 3.14159265358979f;

// Approximate sine and cosine functions
// https://stackoverflow.com/questions/18662261/fastest-implementation-of-sine-cosine-and-square-root-in-c-doesnt-need-to-b
static float sine(float x)
{
  const float B = 4 / PI_F;
  const float C = -4 / (PI_F * PI_F);
  return B * x + C * x * std::abs(x);
}
static float cosine(float x)
{
  return sine(x + (PI_F / 2));
}

// A constant to convert from degrees to radians.
// It maps the random number from [0;UINT32_MAX) to [0;2*pi).
// We do calculations in double precision then convert to float.
constexpr float DEGREE_TO_RADIAN = (2 * PI_D) / UINT32_MAX;

#define SOLUTION
#ifdef SOLUTION
// Simulate the motion of the particles.
// For every particle, we generate a random angle and move the particle
// in the corresponding direction.
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed)
{
  constexpr size_t factor = 8;
  std::array<RNG, factor> rngs{RNG(seed), RNG(seed), RNG(seed), RNG(seed), RNG(seed), RNG(seed), RNG(seed), RNG(seed)};

  const size_t particles_len = particles.size();
  
  for (int j = 0; j < STEPS; j++)
  {
    size_t i = 0;
    for (; i < particles_len - factor; i += factor)
    {
      #pragma unroll 8
      for (size_t k = 0; k < factor; ++k)
      {
        auto &rng = rngs[k];
        auto &p = particles[i + k];
        const uint32_t angle = rng.gen();
        const float angle_rad = angle * DEGREE_TO_RADIAN;
        p.x += cosine(angle_rad) * p.velocity;
        p.y += sine(angle_rad) * p.velocity;
      }
    }

    for (; i < particles_len; ++i)
    {
      auto &rng = rngs[0];
      auto &p = particles[i];
      uint32_t angle = rng.gen();
      float angle_rad = angle * DEGREE_TO_RADIAN;
      p.x += cosine(angle_rad) * p.velocity;
      p.y += sine(angle_rad) * p.velocity;
    }
  }
}

#else

// Simulate the motion of the particles.
// For every particle, we generate a random angle and move the particle
// in the corresponding direction.
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed)
{
  RNG rng(seed);
  for (int i = 0; i < STEPS; i++)
    for (auto &p : particles)
    {
      uint32_t angle = rng.gen();
      float angle_rad = angle * DEGREE_TO_RADIAN;
      p.x += cosine(angle_rad) * p.velocity;
      p.y += sine(angle_rad) * p.velocity;
    }
}

#endif