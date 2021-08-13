
#include "solution.h"
#include <algorithm>

void solution(std::array<S, N>& arr)
{
    std::sort(
        std::begin(arr),
        std::end(arr),
        [](const auto& lhs, const auto& rhs)
        {
            if(lhs.key1 < rhs.key1)
            {
                return true;
            }

            if(lhs.key1 > rhs.key1)
            {
                return false;
            }

            if(lhs.key2 < rhs.key2)
            {
                return true;
            }

            if(lhs.key2 > rhs.key2)
            {
                return false;
            }

            return false;
        });
}
