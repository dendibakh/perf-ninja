#include "solution.h"

#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    int zeros = 0;
    int ones = 0;
    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            zeros++;
        } else if (value == 1) {
            ones++;
        }
    }

    for (int i = 0; i < zeros; i++) {
        array.push_back(std::make_unique<ClassA>());
    }
    for (int i = 0; i < ones; i++) {
        array.push_back(std::make_unique<ClassB>());
    }
    for (std::size_t i = 0; i < N-zeros-ones; i++) {
        array.push_back(std::make_unique<ClassC>());
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
