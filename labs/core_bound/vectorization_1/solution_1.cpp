#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <type_traits>


// My solution.

using score_t = int16_t;

// Types that let us work on 4 elements at the time.
typedef int16_t score_16t __attribute__ ((vector_size (32)));
typedef uint8_t elem_16t __attribute__ ((vector_size (16)));

using column_16t = std::array<score_16t, sequence_size_v + 1>;

score_16t broadcast(score_t value)
{
  return score_16t {
    value, value, value, value,
    value, value, value, value,
    value, value, value, value,
    value, value, value, value
    };
}

score_16t max(score_16t lhs, score_16t rhs)
{
  return lhs > rhs ? lhs : rhs;
}

bool all_true(score_16t values)
{
  return values[0] &&  values[1] &&  values[2] &&  values[3]
    &&   values[4] &&  values[5] &&  values[6] &&  values[7]
    &&   values[8] &&  values[9] && values[10] && values[11]
    &&  values[12] && values[13] && values[14] && values[15];
}

std::ostream& operator<<(std::ostream& stream, score_16t value)
{
  stream << "{"
    <<  value[0] << ", " <<  value[1] << ", " <<  value[2] << ", " <<  value [3]
    <<  value[4] << ", " <<  value[5] << ", " <<  value[6] << ", " <<  value [7]
    <<  value[8] << ", " <<  value[9] << ", " << value[10] << ", " << value [11]
    << value[12] << ", " << value[13] << ", " << value[14] << ", " << value [15]
    << "}";
  return stream;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2)
{
    result_t result {};

    /*
        Initialize score values.
    */
    const score_16t gap_open {broadcast(-11)};
    const score_16t gap_extension{broadcast(-1)};
    const score_16t match{broadcast(6)};
    const score_16t mismatch{broadcast(-4)};

    /*
        Setup the matrix.
        Note we can compute the entire matrix with just one column in memory,
        since we are only interested in the last value of the last column in the
        score matrix.
    */
    column_16t score_column {};
    column_16t horizontal_gap_column {};
    score_16t last_vertical_gap {};

    /*
        Initialize the first column of the matrix.
    */
    horizontal_gap_column[0] = gap_open;
    last_vertical_gap = gap_open;

    for (size_t i = 1; i < sequence_size_v; ++i)
    {
        score_column[i] = last_vertical_gap;
        horizontal_gap_column[i] = last_vertical_gap + gap_open;
        last_vertical_gap += gap_extension;
    }

    /*
        Compute the main recursion to fill the matrix.
    */
    for (unsigned col = 1; col <= sequence_size_v; ++col)
    {
        // Cache last diagonal score to compute this cell.
        score_16t last_diagonal_score = score_column[0];

        score_column[0] = horizontal_gap_column[0];
        last_vertical_gap = horizontal_gap_column[0] + gap_open;
        horizontal_gap_column[0] += gap_extension;

        for (unsigned row = 1; row <= sequence_size_v; ++row)
        {
            elem_16t elems1 = {
                sequences1[0][row - 1],
                sequences1[1][row - 1],
                sequences1[2][row - 1],
                sequences1[3][row - 1],
                sequences1[4][row - 1],
                sequences1[5][row - 1],
                sequences1[6][row - 1],
                sequences1[7][row - 1],
                sequences1[8][row - 1],
                sequences1[9][row - 1],
                sequences1[10][row - 1],
                sequences1[11][row - 1],
                sequences1[12][row - 1],
                sequences1[13][row - 1],
                sequences1[14][row - 1],
                sequences1[15][row - 1]
                };
            elem_16t elems2 = {
                sequences2[0][col - 1],
                sequences2[1][col - 1],
                sequences2[2][col - 1],
                sequences2[3][col - 1],
                sequences2[4][col - 1],
                sequences2[5][col - 1],
                sequences2[6][col - 1],
                sequences2[7][col - 1],
                sequences2[8][col - 1],
                sequences2[9][col - 1],
                sequences2[10][col - 1],
                sequences2[11][col - 1],
                sequences2[12][col - 1],
                sequences2[13][col - 1],
                sequences2[14][col - 1],
                sequences2[15][col - 1]
            };

            // Compute the score from diagonal direction with match/mismatch.
            elem_16t equal_mask = elems1 == elems2;
            score_16t equal_mask_wide {
                (score_t)equal_mask[0],
                (score_t)equal_mask[1],
                (score_t)equal_mask[2],
                (score_t)equal_mask[3],
                (score_t)equal_mask[4],
                (score_t)equal_mask[5],
                (score_t)equal_mask[6],
                (score_t)equal_mask[7],
                (score_t)equal_mask[8],
                (score_t)equal_mask[9],
                (score_t)equal_mask[10],
                (score_t)equal_mask[11],
                (score_t)equal_mask[12],
                (score_t)equal_mask[13],
                (score_t)equal_mask[14],
                (score_t)equal_mask[15],
            };
            score_16t best_cell_score =
                last_diagonal_score + (equal_mask_wide ? match : mismatch);

            // I would like to do this instead of the line above, but getting the
            // following compiler error:
            //   vector condition type (vector of 4 'char' values) and result type
            //   (vector of 4 'int16_t' values) do not have elements of the same size
            // which is true, but for scalar code the condition in a conditional
            // operator doesn't need to be the same size as the return type.
            #if 0
            score_4t best_cell_score = last_diagonal_score + (elems1 == elems2 ? match : mismatch);
            #endif

            // Determine best core from diagonal, vertical, or horizontal direction.
            best_cell_score = max(best_cell_score, last_vertical_gap);
            best_cell_score = max(best_cell_score, horizontal_gap_column[row]);

            // Cache next diagonal value and store optimum in score_column.
            last_diagonal_score = score_column[row];
            score_column[row] = best_cell_score;

            // Compute the next values for vertical and horizontal gap.
            best_cell_score += gap_open;
            last_vertical_gap += gap_extension;
            horizontal_gap_column[row] += gap_extension;

            // Store optimum between gap open and gap extension.
            last_vertical_gap = max(last_vertical_gap, best_cell_score);
            horizontal_gap_column[row] = max(horizontal_gap_column[row], best_cell_score);
        }
    }

    // Report back best score.
    score_16t scores = score_column.back();
    result[0] = scores[0];
    result[1] = scores[1];
    result[2] = scores[2];
    result[3] = scores[3];
    result[4] = scores[4];
    result[5] = scores[5];
    result[6] = scores[6];
    result[7] = scores[7];
    result[8] = scores[8];
    result[9] = scores[9];
    result[10] = scores[10];
    result[11] = scores[11];
    result[12] = scores[12];
    result[13] = scores[13];
    result[14] = scores[14];
    result[15] = scores[15];

    return result;
}
