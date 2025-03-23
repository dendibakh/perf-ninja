#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

using simd_sequence = std::array<std::array<uint8_t, sequence_count_v>, sequence_size_v>;
simd_sequence get_transpose(const std::vector<sequence_t>& seq )
{
    simd_sequence result;
    for (int i = 0; i < sequence_count_v; ++i)
        for (int j = 0; j<sequence_size_v; ++j)
            result[j][i] = seq[i][j];
    return result;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  using score_t = int16_t;
  using simd_column_t = std::array<std::array<score_t, sequence_count_v>, sequence_size_v+1>;

  /*
   * Initialise score values.
   */
  static constexpr score_t gap_open{-11};
  static constexpr score_t gap_extension{-1};
  static constexpr score_t match{6};
  static constexpr score_t mismatch{-4};

  simd_sequence seqs1 = get_transpose(sequences1), seqs2 = get_transpose(sequences2);

  /*
   * Setup the matrix.
   * Note we can compute the entire matrix with just one column in memory,
   * since we are only interested in the last value of the last column in the
   * score matrix.
   */
  simd_column_t score_column{};
  simd_column_t horizontal_gap_column{};
  std::array<score_t, sequence_count_v> last_vertical_gap{};

  /*
   * Initialise the first column of the matrix.
   */
  for (int i = 0; i<sequence_count_v; ++i)
  {
    horizontal_gap_column[0][i] = gap_open;
    last_vertical_gap[i] = gap_open;
  }

  for (size_t i = 1; i < score_column.size(); ++i) {
    for (int j = 0; j<sequence_count_v; ++j)
    {
      score_column[i][j] = last_vertical_gap[j];
      horizontal_gap_column[i][j] = last_vertical_gap[j] + gap_open;
      last_vertical_gap[j] += gap_extension;
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= seqs1.size(); ++col) {
    std::array<score_t, sequence_count_v> last_diagonal_score{};
    for (int i = 0; i<sequence_count_v; ++i)
    {
      last_diagonal_score[i] = score_column[0][i]; // Cache last diagonal score to compute this cell.
      score_column[0][i] = horizontal_gap_column[0][i];
      last_vertical_gap[i] = horizontal_gap_column[0][i] + gap_open;
      horizontal_gap_column[0][i] += gap_extension;
    }

    for (unsigned row = 1; row <= seqs1.size(); ++row) {
      for (int i = 0; i<sequence_count_v; ++i)
      {
        score_t best_cell_score =
            last_diagonal_score[i] +
            (seqs1[row - 1][i] == seqs2[col - 1][i] ? match : mismatch);

        best_cell_score = std::max(best_cell_score, last_vertical_gap[i]);
        best_cell_score = std::max(best_cell_score, horizontal_gap_column[row][i]);
        last_diagonal_score[i] = score_column[row][i];
        score_column[row][i] = best_cell_score;

        best_cell_score += gap_open;
        last_vertical_gap[i] += gap_extension;
        horizontal_gap_column[row][i] += gap_extension;

        last_vertical_gap[i] = std::max(last_vertical_gap[i], best_cell_score);
        horizontal_gap_column[row][i] =
            std::max(horizontal_gap_column[row][i], best_cell_score);
      }
    }
  }

  result_t result = score_column.back();

  return result;
}
