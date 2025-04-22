#include "solution.hpp"

#include <algorithm>


// Force inline
#define INLINE inline __attribute__((always_inline))

// min
template <typename T> INLINE const T &min(const T &x, const T &y)
{
    return (x < y) ? x : y;
}

// inplace matrix transpose would probably faster than creating a new matrix for out???


void transposeCacheOblivious(int x0, int y0, int dx, int dy, int blockSize, MatrixOfDoubles &in, MatrixOfDoubles &out)
{ // ref: https://github.com/dedlocc/cache-oblivious/blob/master/src/Matrix.tpp
    if (dx <= blockSize && dy <= blockSize) {
        auto x1 = x0 + dx;
        auto y1 = y0 + dy;
        for (int i = x0; i < x1; ++i) {
          for (int j = y0; j < y1; ++j) {
                out[j][i] = in[i][j];
            }
        }
        return;
    }

    if (dy <= dx) {
        auto mid = dx / 2;
        transposeCacheOblivious(x0, y0, mid, dy, blockSize, in, out);
        transposeCacheOblivious(x0 + mid, y0, dx - mid, dy, blockSize, in, out);
        return;
    }

    auto mid = dy / 2;
    transposeCacheOblivious(x0, y0, dx, mid, blockSize, in, out);
    transposeCacheOblivious(x0, y0 + mid, dx, dy - mid, blockSize, in, out);
}



bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out)
{
  static constexpr int BLOCK_SIZE{250}; // 8.3ms less overhead from function calls with bigger blocksize too
    int size = in.size();

    transposeCacheOblivious(0, 0,  size, size, BLOCK_SIZE, in, out);

    // tile solution was 12ms, probably faster for smaller matrices though:
    //---------------------
    // static constexpr int TILE_SIZE{8};
    // for (int ii = 0; ii < size; ii += TILE_SIZE)
    // {
    //     int ib = min(ii + TILE_SIZE, size);

    //     for (int jj = 0; jj < size; jj += TILE_SIZE)
    //     {
    //         int jb = min(jj + TILE_SIZE, size);

    //         for (int i = ii; i < ib; i++)
    //         {
    //             for (int j = jj; j < jb; j++)
    //             {
    //                 out[j][i] = in[i][j];
    //             }
    //         }
    //     }
    // }

    return out[0][size - 1];
}

// bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out)
// {
//     int size = in.size();
//     for (int i = 0; i < size; i++)
//     {
//         for (int j = 0; j < size; j++)
//         {
//             out[i][j] = in[j][i];
//         }
//     }
//     return out[0][size - 1];
// }
