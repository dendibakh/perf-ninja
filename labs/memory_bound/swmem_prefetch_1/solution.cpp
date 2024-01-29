#include "solution.hpp"

static int getSumOfDigits(int n) {
    int sum = 0;
    while (n != 0) {
        sum = sum + n % 10;
        n = n / 10;
    }
    return sum;
}

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
    int result = 0;
    const int PrefetchWindow = 150;

    for (int i = 0; i < lookups.size(); i++) {
        auto val = lookups[i];
        if (i + PrefetchWindow < lookups.size())
            hash_map->prefetch(lookups[i + PrefetchWindow]);

        if (hash_map->find(val))
            result += getSumOfDigits(val);
    }

    return result;
}
