#include <vector>
#include <iostream>
#include <cstdint>
#include <array>

// The number of motion simulation steps.
constexpr uint32_t STEPS = 10000;
// The number of paticles to simulate.
constexpr uint32_t PARTICLES = 1000;

struct Particle {
  float x;
  float y;
  //float hz;
  float velocity;
};

// Initialize the particles with random coordinates and velocities.
std::vector<Particle> initParticles();

// Medium-quality random number generator.
// https://www.javamex.com/tutorials/random_numbers/xorshift.shtml
struct XorShift322 {
  uint32_t val;
  XorShift322 (uint32_t seed) : val(seed) {}
public:
  uint32_t gen() {
    val ^= (val << 13);
    val ^= (val >> 17);
    val ^= (val << 5);
    return val;
  }
};

struct XorShift32 {
  std::array<uint32_t,4> vals;
  int idx = 0;
  XorShift32 (uint32_t seed) {

    for (auto& val : vals) {
      val = seed++;
      val ^= (val << 13);
    }
    for (auto& val : vals) {
      val ^= (val >> 17);
    }
    for (auto& val : vals) {
      val ^= (val << 5);
    }
  }
public:
  uint32_t gen() {
    if (idx == 4) {
      idx = 0;
      for (auto& val : vals) {
        val ^= (val << 13);
      }
      for (auto& val : vals) {
        val ^= (val >> 17);
      }
      for (auto& val : vals) {
        val ^= (val << 5);
      }
    }


    return vals[idx++];
  }
};

constexpr double PI_D = 3.141592653589793238463;
constexpr float PI_F = 3.14159265358979f;

// Approximate sine and cosine functions
// https://stackoverflow.com/questions/18662261/fastest-implementation-of-sine-cosine-and-square-root-in-c-doesnt-need-to-b
inline float sine(float x) {
    constexpr float B = 4 / PI_F;
    constexpr float C = -4/( PI_F * PI_F);
    float bx = B*x;
    float cx = C * x * x;
    return bx + cx;
}
inline float cosine(float x) {
    return sine(x + (PI_F / 2));
}

inline std::pair<float,float> sincos_approx(float x) {
  constexpr float B  = 4.0f/PI_F;
  constexpr float C  = -4.0f/(PI_F*PI_F);
  constexpr float Y  = PI_F * 0.5f;
  constexpr float K1 = B * Y + C * Y * Y;
  constexpr float K2 = 2.0f * C * Y;

  float x2 = x * x;
  float s  = B * x + C * x2;
  float c  = s + K1 + K2 * x;
  return { s, c };
}
// A constant to convert from degrees to radians.
// It maps the random number from [0;UINT32_MAX) to [0;2*pi).
// We do calculations in double precision then convert to float.
constexpr float DEGREE_TO_RADIAN = (2 * PI_D) / UINT32_MAX;

// Simulate the motion of the particles.
// For every particle, we generate a random angle and move the particle
// in the corresponding direction.
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {
  RNG rng(seed);
  int k = 0;
  for (int i = 0; i < STEPS; i++)
    for (k = 0; k< PARTICLES; k+=4) {
      Particle& p1 = particles[k];
      Particle& p2 = particles[k+1];
      Particle& p3 = particles[k+2];
      Particle& p4 = particles[k+3];
      float v1 = p1.velocity;
      float v2 = p2.velocity;
      float v3 = p3.velocity;
      float v4 = p4.velocity;
      uint32_t angle1 = rng.gen();
      uint32_t angle2 = rng.gen();
      uint32_t angle3 = rng.gen();
      uint32_t angle4 = rng.gen();

      float angle_rad1 = angle1 * DEGREE_TO_RADIAN;
      float angle_rad2 = angle2 * DEGREE_TO_RADIAN;
      float angle_rad3 = angle3 * DEGREE_TO_RADIAN;
      float angle_rad4 = angle4 * DEGREE_TO_RADIAN;

      p1.x += cosine(angle_rad1) * v1;
      p1.y += sine(angle_rad1) * v1;
      p2.x += cosine(angle_rad2) * v2;
      p2.y += sine(angle_rad2) * v2;
      p3.x += cosine(angle_rad3) * v3;
      p3.y += sine(angle_rad3) * v3;
      p4.x += cosine(angle_rad4) * v4;
      p4.y += sine(angle_rad4) * v4;
    }

}