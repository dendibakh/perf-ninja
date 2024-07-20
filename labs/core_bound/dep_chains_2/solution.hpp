#include <vector>
#include <iostream>
#include <cstdint>
#include <array>
// #include <immintrin.h>

// The number of motion simulation steps.
constexpr uint32_t STEPS = 10000;
// The number of paticles to simulate.
constexpr uint32_t PARTICLES = 1000;

struct Particle {
  float x;
  float y;
  float velocity;
};

// Initialize the particles with random coordinates and velocities.
std::vector<Particle> initParticles();

// Medium-quality random number generator.
// https://www.javamex.com/tutorials/random_numbers/xorshift.shtml
struct XorShift32 {
  uint32_t val;
  XorShift32 (uint32_t seed) : val(seed) {}
public:
  uint32_t gen() {
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
static float sine(float x) {
    const float B = 4 / PI_F;
    const float C = -4/( PI_F * PI_F);
    return B * x + C * x * std::abs(x);
}
static float cosine(float x) {
    return sine(x + (PI_F / 2));
}

// static __m256 abs(__m256 x) {
//   static const __m256 abs_mask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));
//   return _mm256_and_ps(x, abs_mask);
// }

// static __m256 sine(__m256 x) {
//   static const __m256 B = _mm256_set1_ps(4.0 / PI_F);
//   static const __m256 C = _mm256_set1_ps(-4.0 / (PI_F * PI_F));
//   const __m256 abs_x = abs(x); 
//   __m256 y = _mm256_mul_ps(B, x);
//   __m256 z = _mm256_mul_ps(C, x);
//   z = _mm256_mul_ps(z, abs_x);
//   return _mm256_add_ps(y, z);
// }

// static __m256 cosine(__m256 x) {
//   static const __m256 B = _mm256_set1_ps(PI_F / 2);
//   x = _mm256_add_ps(x, B);
//   return sine(x);
// }

// A constant to convert from degrees to radians.
// It maps the random number from [0;UINT32_MAX) to [0;2*pi).
// We do calculations in double precision then convert to float.
constexpr float DEGREE_TO_RADIAN = (2 * PI_D) / UINT32_MAX;
static constexpr int kBatch = 256 / 32;
// Simulate the motion of the particles.
// For every particle, we generate a random angle and move the particle
// in the corresponding direction.
template <class RNG>
void randomParticleMotion(std::vector<Particle> &particles, uint32_t seed) {

  std::vector<RNG> rngs;
  for (int i = 0; i < kBatch; i++) {
    rngs.emplace_back(seed);
  }

  // for (int i = 0; i < STEPS; i++)
  //   for (auto &p : particles) {
  //     uint32_t angle = rng.gen();
  //     float angle_rad = angle * DEGREE_TO_RADIAN;
  //     p.x += cosine(angle_rad) * p.velocity;
  //     p.y += sine(angle_rad) * p.velocity;
  //   }

  // return;

  int N = particles.size();
  uint32_t degree_angles[N];
  float px[N];
  float py[N];
  float pv[N];

  for (int i = 0; i < N; i++) {
    px[i] = particles[i].x;
    py[i] = particles[i].y;
    pv[i] = particles[i].velocity;
  }

  // static const __m256 degree_to_radian_bc = _mm256_set1_ps(DEGREE_TO_RADIAN);
  for (int i = 0; i < STEPS; i++) {
    int t = 0;
    for (int j = 0; j < N; j++) {
      degree_angles[j] = rngs[j % kBatch].gen();
    }
    // for (; t + kBatch - 1 < N; t += kBatch) {
    //   __m256i r_angle = _mm256_loadu_si256((__m256i*)(degree_angles + t));
    //   __m256 angle = _mm256_mul_ps(degree_to_radian_bc, _mm256_cvtepi32_ps(r_angle));
    //   __m256 batch_x = _mm256_loadu_ps(px + t);
    //   __m256 batch_y = _mm256_loadu_ps(py + t);
    //   __m256 batch_v = _mm256_loadu_ps(pv + t);

    //   auto batch_sine = sine(angle);
    //   auto batch_cosine = cosine(angle);
    //   auto mul_batch_sine = _mm256_mul_ps(batch_sine, batch_v);
    //   auto mul_batch_cosine = _mm256_mul_ps(batch_cosine, batch_v);

    //   batch_x = _mm256_add_ps(batch_x, mul_batch_cosine);
    //   batch_y = _mm256_add_ps(batch_y, mul_batch_sine);

    //   _mm256_storeu_ps(px + t, batch_x);
    //   _mm256_storeu_ps(py + t, batch_y);
    // }

    for (; t < particles.size(); t++) {
      float angle_rad = degree_angles[t] * DEGREE_TO_RADIAN;
      px[t] += cosine(angle_rad) * pv[t];
      py[t] += sine(angle_rad) * pv[t];
    }
  }

  for (int i = 0; i < N; i++) {
    particles[i].x = px[i];
    particles[i].y = py[i];
  }
}