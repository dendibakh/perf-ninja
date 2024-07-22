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


    int i = 0;
    const int prefetch_dist = 16;
    for (; i + prefetch_dist < lookups.size(); ++i) {
        hash_map->prefetch(lookups[i + prefetch_dist]);
        if (hash_map->find(lookups[i]))
            result += getSumOfDigits(lookups[i]);
    }
    for (; i < lookups.size(); ++i) {
        if (hash_map->find(lookups[i]))
            result += getSumOfDigits(lookups[i]);
    }

    return result;
}
