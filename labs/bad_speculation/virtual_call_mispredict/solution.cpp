#include "solution.h"

#include <random>
#include <array>
#define SOLUTION
#ifdef SOLUTION
void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);
    std::array<std::uint32_t, N > indices;
    std::array<std::size_t, 3> types = {0,0,0};
    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        indices[i] = value;
        types[value]++;
    }
    array.resize(N);
    std::array<std::size_t, 3> curr = {0, types[0], types[0] + types[1]};
    for (std::size_t i = 0; i < N; i++) {
        int value = indices[i];
        if (value == 0) {
            array[curr[value]] = std::make_unique<ClassA>();
            curr[value]++;
        } else if (value == 1) {
            array[curr[value]] = std::make_unique<ClassB>();
            curr[value]++;
        } else {
            array[curr[value]] = std::make_unique<ClassC>();
            curr[value]++;
        }
    }
}
#else
void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            array.push_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            array.push_back(std::make_unique<ClassB>());
        } else {
            array.push_back(std::make_unique<ClassC>());
        }
    }
}
#endif
// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
