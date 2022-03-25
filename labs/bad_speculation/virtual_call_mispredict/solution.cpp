#include "solution.h"

#include <algorithm>
#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    std::vector<uint32_t> values;
    values.reserve(N);
    for (std::size_t i = 0; i < N; i++) {
        values.push_back(distribution(generator));
    }
    std::sort(values.begin(), values.end());
    for (const auto value : values) {
        if (value == 0) {
            array.push_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            array.push_back(std::make_unique<ClassB>());
        } else {
            array.push_back(std::make_unique<ClassC>());
        }
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
