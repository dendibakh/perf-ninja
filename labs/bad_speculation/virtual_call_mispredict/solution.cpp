#include "solution.h"

#include <random>
#include <algorithm>
#include <iterator>

// push back classes in order, to avoid cache misses

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    std::array<std::vector<std::unique_ptr<BaseClass>>, 3> classes;
    // Pre-allocate memory
    for (auto& classVector: classes) {
        classVector.reserve(N / 3);
    }

    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            classes[0].emplace_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            classes[1].emplace_back(std::make_unique<ClassB>());
        } else {
            classes[2].emplace_back(std::make_unique<ClassC>());
        }
    }

    for (auto& classVector: classes) {
        std::move(classVector.begin(), classVector.end(), std::back_inserter(array));
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {

    for (const auto& item: array) {
        item->handle(data);
    }
}
