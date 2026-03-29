#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

using simd_sequence_t = std::array<uint8_t, sequence_count_v>;
using simd_vector_t = std::array<simd_sequence_t, sequence_size_v>;

simd_vector_t transpose(std::vector<sequence_t> const &sequences)
{
  simd_vector_t transposed{};

  for (size_t i = 0; i < sequence_size_v; ++i)
    for (size_t j = 0; j < sequence_count_v; ++j)
      transposed[i][j] = sequences[j][i];
  return transposed;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1, std::vector<sequence_t> const &sequences2)
{
  result_t result{};
  auto tr_seq1 = transpose(sequences1);
  auto tr_seq2 = transpose(sequences2);

  {
    using score_t = std::array<int16_t, sequence_count_v>;
    using column_t = std::array<score_t, sequence_size_v + 1>;

    score_t gap_open;
    gap_open.fill(-11);
    score_t gap_extension;
    gap_extension.fill(-1);
    score_t match;
    match.fill(6);
    score_t mismatch;
    mismatch.fill(-4);

    column_t score_column{};
    column_t horizontal_gap_column{};
    score_t last_vertical_gap{};

    for (size_t k = 0; k < sequence_count_v; ++k)
    {
      horizontal_gap_column[0][k] = gap_open[k];
      last_vertical_gap[k] = gap_open[k];
    }

    for (size_t i = 1; i < score_column.size(); ++i)
    {
      for (size_t k = 0; k < sequence_count_v; ++k)
      {
        score_column[i][k] = last_vertical_gap[k];
        horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open[k];
        last_vertical_gap[k] += gap_extension[k];
      }
    }

    for (unsigned col = 1; col <= sequence_size_v; ++col)
    {
      score_t last_diagonal_score{};
      for (size_t k = 0; k < sequence_count_v; ++k)
      {
        last_diagonal_score[k] = score_column[0][k]; // Cache last diagonal score to compute this cell.
        score_column[0][k] = horizontal_gap_column[0][k];
        last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open[k];
        horizontal_gap_column[0][k] += gap_extension[k];
      }

      for (unsigned row = 1; row <= sequence_size_v; ++row)
      {
        score_t best_cell_score = last_diagonal_score;
        for (size_t k = 0; k < sequence_count_v; ++k)
        {
          best_cell_score[k] += (tr_seq1[row - 1][k] == tr_seq2[col - 1][k] ? match[k] : mismatch[k]);

          best_cell_score[k] = std::max(best_cell_score[k], last_vertical_gap[k]);
          best_cell_score[k] = std::max(best_cell_score[k], horizontal_gap_column[row][k]);

          last_diagonal_score[k] = score_column[row][k];
          score_column[row][k] = best_cell_score[k];

          best_cell_score[k] += gap_open[k];
          last_vertical_gap[k] += gap_extension[k];
          horizontal_gap_column[row][k] += gap_extension[k];

          last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score[k]);
          horizontal_gap_column[row][k] = std::max(horizontal_gap_column[row][k], best_cell_score[k]);
        }
      }
    }

    for (size_t k = 0; k < sequence_count_v; ++k)
      result[k] = score_column.back()[k];
  }

  return result;
}
