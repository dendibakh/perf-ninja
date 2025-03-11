// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include "solution.h"
#include <random>

S create_entry(const int first_value, const int second_value) {
    return {
        static_cast<float>(first_value) / maxRandom,
        static_cast<long long>(first_value * second_value),
        static_cast<char>(second_value),
        static_cast<char>(second_value),
        first_value < second_value
    };
}

void init(std::array<S, N> &arr) {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(minRandom, maxRandom - 1);

    for (int i = 0; i < N; i++) {
        const int random_int1 = distribution(generator);
        const int random_int2 = distribution(generator);

        arr[i] = create_entry(random_int1, random_int2);
    }
}
