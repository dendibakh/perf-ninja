include(CheckCXXSourceRuns)

# https://stackoverflow.com/questions/11228855/header-files-for-x86-simd-intrinsics
if(MSVC)
  set(CMAKE_REQUIRED_FLAGS "/arch:AVX512")
  check_cxx_source_runs("
    #include <cstdlib>
    #include <immintrin.h>
    int main() {
      __m512i v = _mm512_set1_epi32(-1);
      v = _mm512_add_epi32(v, v);
      return (_mm_cvtsi128_si32(_mm512_extracti32x4_epi32(v, 3)) == -2) ? EXIT_SUCCESS : EXIT_FAILURE;
    }"
    SUPPORT_MSVC_AVX512)

  set(CMAKE_REQUIRED_FLAGS "/arch:AVX2")
  check_cxx_source_runs("
    #include <cstdlib>
    #include <immintrin.h>
    int main() {
      __m256i v = _mm256_set1_epi32(-1);
      v = _mm256_hadd_epi32(v, v);
      return (_mm_cvtsi128_si32(_mm256_extracti128_si256(v, 1)) == -2) ? EXIT_SUCCESS : EXIT_FAILURE;
    }"
    SUPPORT_MSVC_AVX2)

  set(CMAKE_REQUIRED_FLAGS "/arch:AVX")
  check_cxx_source_runs("
    #include <cstdlib>
    #include <immintrin.h>
    int main() {
      __m256 v = _mm256_set1_ps(-3.0f);
      v = _mm256_mul_ps(v, v);
      return (_mm_cvtss_f32(_mm256_extractf128_ps(v, 1)) == 9.0f) ? EXIT_SUCCESS : EXIT_FAILURE;
    }"
    SUPPORT_MSVC_AVX)

  set(CMAKE_REQUIRED_FLAGS "")
endif()
