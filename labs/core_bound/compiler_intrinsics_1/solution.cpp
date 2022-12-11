
#include "solution.h"
#include <memory>

#if defined(ON_MACOS)
#include "sse2neon.h" // https://github.com/DLTcollab/sse2neon
#elif defined(ON_LINUX) || defined(ON_WINDOWS)
#include "xmmintrin.h"
#include "smmintrin.h"
#include "immintrin.h"
#include "mmintrin.h" // https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
#endif

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output) {
  int pos = 0;
  int currentSum = 0;
  int size = static_cast<int>(input.size());

  // 1. left border - time spend in this loop can be ignored, no need to
  // optimize it
  for (int i = 0; i < std::min<int>(size, radius); ++i) {
    currentSum += input[i];
  }

  // The input vector size is 40000
  // The maximum value of radius is 255
  // The radius is a constant 13
  // The output vector can be up to 40000
  int limit = std::min(radius + 1, size - radius);
  
  // So the limit is 14

  for (pos = 0; pos < limit; ++pos) {
    // Use intrinsic to add 14 eleme
    currentSum += input[pos + radius];
    // The sum of the first 14 elements is stored in currentSum
    // For each iteration the output vector is updated with the currentSum
    output[pos] = currentSum;
  }

  // 2. main loop.
  limit = size - radius;

  // we widen each 8 bit integer to 16 bit, we do that because the output vector is 16 bit
  // The code calculates the prefix sum for 8 elements at a time
  // by using the SSE intrinsics. The prefix sum is the running sum
  // for each element in the vector. For example, if the input vector
  // is [a, b, c, d, e, f, g, h], the prefix sum is [a, a+b, a+b+c, a+b+c+d,
  // a+b+c+d+e, a+b+c+d+e+f, a+b+c+d+e+f+g, a+b+c+d+e+f+g+h]. This is done
  // by calculating the diff between the add and sub vectors, and then
  // adding the diff to the previous prefix sum. The diff vector is
  // shifted left 2 bytes, 1 element, 4 bytes, 2 elements, and 8 bytes,
  // 4 elements to calculate the prefix sum for the 8 elements. The
  // result is then added to the current prefix sum.
  // The function takes in two parameters:
  // 1. inputPtr: The pointer to the input array
  // 2. outputPtr: The pointer to the output array
  // The function returns the prefix sum of the last element in the array
  // in the output array. This is used for the next iteration.

  // SSE4.1 Solution

  // Will be used to access the elements in the input array that need to be substracted from each other
  const uint8_t* substractPtr = input.data() + pos - radius - 1;

  // Will be used to access the elements in the input array that need to be added to each other
  const uint8_t* addPtr = input.data() + pos + radius;
  
  // Store the resulting prefix sum values
  const uint16_t* outputPtr = output.data() + pos;

  // Initialize to the currentSum value. Will be used to store the intermediate results
  __m128i current = _mm_set1_epi16(currentSum);

  int i = 0;
  // Process up to 8 elements at a time
  for(; i + 7 < limit - pos; i += 8)
  {
    // Calculate vector difference: input[i+radius] - input[i-radius-1]
    // Load 64bit, so 8, 8 bit integers into a vector that we can substract 
    __m128i substract_u8 = _mm_loadu_si64(substractPtr + i);

    // we widen each 8 bit integer to 16 bit
    __m128i sub = _mm_cvtepu8_epi16(substract_u8);

    // Load 64bit, so 8, 8 bit integers into a vector that we can add
    __m128i add_u8 = _mm_loadu_si64(addPtr + i);

    // we widen each 8 bit integer to 16 bit, we do that because the output vector is 16 bit
    __m128i add = _mm_cvtepu8_epi16(add_u8);

    // Calculate the diff between the add and sub vectors
    __m128i diff = _mm_sub_epi16(add, sub);

    // Calculate vector prefix sum for 8 elements

    // We shift the diff vector to the left by 2 bytes, 1 element
    __m128i prefixSum = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));

    // We shift the diff vector to the left by 4 bytes, 2 elements
    prefixSum = _mm_add_epi16(prefixSum, _mm_slli_si128(prefixSum, 4));

    // We shift the diff vector to the left by 8 bytes, 4 elements
    // We do left shift because of the endianness
    prefixSum = _mm_add_epi16(prefixSum, _mm_slli_si128(prefixSum, 8));

    // Store the result
    __m128i result = _mm_add_epi16(prefixSum, current);
    _mm_storeu_si128((__m128i*)(outputPtr + i), result);

    // Broadcast currentSum for the next iteration
    currentSum = (int16_t)_mm_extract_epi16(result, 7);
    current = _mm_set1_epi16(currentSum);
  }

  pos += i;

  // Still need to process the remaining elements
  for(; pos < limit; ++pos)
  {
    currentSum += input[pos + radius];
    currentSum -= input[pos - radius - 1];
    output[pos] = currentSum;
  }

  // 3. special case, executed only if size <= 2*radius + 1
  limit = std::min(radius + 1, size);
  for (; pos < limit; pos++) {
    output[pos] = currentSum;
  }

  // 4. right border - time spend in this loop can be ignored, no need to
  // optimize it
  for (; pos < size; ++pos) {
    currentSum -= input[pos - radius - 1];
    output[pos] = currentSum;
  }
}
