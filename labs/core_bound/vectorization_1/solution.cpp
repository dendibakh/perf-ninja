#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

#include <immintrin.h>

inline constexpr size_t vw = 32;
inline constexpr size_t vss = sequence_size_v / vw + 1;

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};

  for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
       ++sequence_idx) {
    using score_t = int16_t;
    using vs_t = __m512i;
    using column_t = std::array<vs_t, vss>;

    auto get2 = [](vs_t &a, unsigned i) {
      __m128i quarter;
      switch (i/8) {
        case 0: quarter = _mm512_extracti32x4_epi32(a, 0); break;
        case 1: quarter = _mm512_extracti32x4_epi32(a, 1); break;
        case 2: quarter = _mm512_extracti32x4_epi32(a, 2); break;
        default: quarter = _mm512_extracti32x4_epi32(a, 3); break;
      }
      switch (i%8) {
        case 0: return (score_t)_mm_extract_epi16(quarter, 0);
        case 1: return (score_t)_mm_extract_epi16(quarter, 1);
        case 2: return (score_t)_mm_extract_epi16(quarter, 2);
        case 3: return (score_t)_mm_extract_epi16(quarter, 3);
        case 4: return (score_t)_mm_extract_epi16(quarter, 4);
        case 5: return (score_t)_mm_extract_epi16(quarter, 5);
        case 6: return (score_t)_mm_extract_epi16(quarter, 6);
        default: return (score_t)_mm_extract_epi16(quarter, 7);
      }
    };
    auto get = [get2](column_t &a, unsigned i) {
      return get2(a[i/vw], i%vw);
    };
    auto set2 = [](vs_t &a, unsigned i, score_t x) {
      a = _mm512_mask_set1_epi16(a, 1u<<i, x);
    };
    auto set = [](column_t &a, unsigned i, score_t x) {
      a[i/vw] = _mm512_mask_set1_epi16(a[i/vw], 1u<<i%vw, x);
    };
    vs_t perm[5];
    for (unsigned k=0; k<5; ++k)
      for (unsigned i=0; i<vw; ++i) 
        set2(perm[k], i, (int16_t)(i<(1<<k)?0:i-(1<<k)));
    auto butterfly = [perm](vs_t &a) {
      for (unsigned i=0; i<5; ++i)
        a = _mm512_max_epi16(_mm512_permutexvar_epi16(perm[i], a), a);
    };

    sequence_t const &sequence1 = sequences1[sequence_idx];
    sequence_t const &sequence2 = sequences2[sequence_idx];

    /*
     * Initialise score values.
     */
    score_t gap_open{-11};
    score_t gap_extension{-1};
    score_t match{6};
    score_t mismatch{-4};

    vs_t vert_gap_1;
    for (unsigned i=0; i<vw; ++i)
      set2(vert_gap_1, i, gap_open+(vw-1-i)*gap_extension);
    vs_t vert_gap_2;
    for (unsigned i=0; i<vw; ++i)
      set2(vert_gap_2, i, (i-vw)*gap_extension);
    
    /*
     * Setup the matrix.
     * Note we can compute the entire matrix with just one column in memory,
     * since we are only interested in the last value of the last column in the
     * score matrix.
     */
    column_t score_column{};
    column_t horizontal_gap_column{};
    score_t last_vertical_gap{};

    /*
     * Initialise the first column of the matrix.
     */
    set(horizontal_gap_column, 0, gap_open);
    last_vertical_gap = gap_open;

    for (size_t i = 1; i <= sequence_size_v; ++i) {
      set(score_column, i, last_vertical_gap);
      set(horizontal_gap_column, i, last_vertical_gap + gap_open);
      last_vertical_gap += gap_extension;
    }

    /*
     * Compute the main recursion to fill the matrix.
     */
    for (unsigned col = 1; col <= sequence2.size(); ++col) {
      score_t last_diagonal_score =
          get(score_column, 0); // Cache last diagonal score to compute this cell.
      set(score_column, 0, get(horizontal_gap_column, 0));

      for (unsigned row = 1; row <= sequence1.size(); ++row) {
        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score =
            last_diagonal_score +
            (sequence1[row - 1] == sequence2[col - 1] ? match : mismatch);
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score = std::max(best_cell_score, get(horizontal_gap_column, row));
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score = get(score_column, row);
        set(score_column, row, best_cell_score);
      }
      last_vertical_gap = get(horizontal_gap_column, 0) + gap_open;
      for (unsigned row = 0; row < vss; ++row) {
        vs_t a = _mm512_add_epi16(score_column[row], vert_gap_1);
        set2(a, 0, std::max(get2(a, 0), (score_t)(last_vertical_gap+gap_extension*vw)));
        butterfly(a);
        last_vertical_gap = get2(a, 31);
        a = _mm512_add_epi16(a, vert_gap_2);
        score_column[row] = _mm512_max_epi16(score_column[row], a);
      }
      for (unsigned row = 0; row < vss; ++row) {
        horizontal_gap_column[row] = _mm512_add_epi16(horizontal_gap_column[row], _mm512_set1_epi16(gap_extension));
        horizontal_gap_column[row] = _mm512_max_epi16(horizontal_gap_column[row], _mm512_add_epi16(score_column[row], _mm512_set1_epi16(gap_open)));
      }
    }

    // Report the best score.
    result[sequence_idx] = get(score_column, sequence_size_v);
  }

  return result;
}
