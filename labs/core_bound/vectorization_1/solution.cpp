#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
    result_t result{};

    for (size_t sequence_idx = 0; sequence_idx < sequences1.size();
         ++sequence_idx) {
        using score_t = int16_t;
        constexpr size_t DiagSize = sequence_size_v + 3;
        using diagonal_t = std::array<score_t, DiagSize>;
        const int MatrixN = sequence_size_v + 1;

        sequence_t const &sequence1 = sequences1[sequence_idx];
        sequence_t const &sequence2 = sequences2[sequence_idx];

        /*
         * Initialise score values.
         */
        score_t gap_open{-11};
        score_t gap_extension{-1};
        score_t match{6};
        score_t mismatch{-4};

        diagonal_t diagonal_score[3] = {};
        diagonal_t diagonal_score_with_v_gap[3] = {};
        diagonal_t diagonal_score_with_h_gap[3] = {};
        const score_t Inf = 10000;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < DiagSize; j++) {
                diagonal_score[i][j] = -Inf;
                diagonal_score_with_v_gap[i][j] = -Inf;
                diagonal_score_with_h_gap[i][j] = -Inf;
            }
        }

        diagonal_score[0][1] = 0;
        diagonal_score[1][1] = diagonal_score[1][2] = gap_open;
        diagonal_score_with_v_gap[1][1] = diagonal_score[1][1];
        diagonal_score_with_v_gap[1][2] = diagonal_score[1][2] + gap_open;

        diagonal_score_with_h_gap[1][2] = diagonal_score[1][2];
        diagonal_score_with_h_gap[1][1] = diagonal_score[1][1] + gap_open;

        for (int diag = 2; diag < MatrixN * 2 - 1; diag++) {
            const int cnt = std::min(diag + 1, 2 * MatrixN - 1 - diag);
            const int c_id = diag % 3;
            const int p_id = (diag - 1 + 3) % 3;
            const int pp_id = (diag - 2 + 3) % 3;

            int start_i = std::max(diag - MatrixN + 1, 0) + 1;

            for (int i = start_i; i < start_i + cnt; i++) {
                int hi = i - 1;
                int vi = i;
                int di = i - 1;
                int s1 = i - 1;
                int s2 = diag - s1;

                diagonal_score_with_h_gap[c_id][i] = std::max(diagonal_score_with_h_gap[p_id][hi] + gap_extension, diagonal_score[p_id][hi] + gap_open);
                diagonal_score_with_v_gap[c_id][i] = std::max(diagonal_score_with_v_gap[p_id][vi] + gap_extension, diagonal_score[p_id][vi] + gap_open);

                auto const new_gap_score= std::max(diagonal_score_with_v_gap[c_id][i], diagonal_score_with_h_gap[c_id][i]);
                diagonal_score[c_id][i] = new_gap_score;

                if (s1 * s2 != 0) {
                    const score_t candidate = diagonal_score[pp_id][di] + (sequence1[s1 - 1] == sequence2[s2 - 1] ? match : mismatch);
                    diagonal_score[c_id][i] = std::max(diagonal_score[c_id][i], candidate);
                }
            }
        }

        // Report the best score.
        result[sequence_idx] = diagonal_score[(MatrixN * 2 - 2) % 3][MatrixN];
    }

    return result;
}
