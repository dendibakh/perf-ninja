#include "solution.h"

#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    std::size_t typeB = 0;

    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            array.push_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            ++typeB;
        }
    }

    for (std::size_t i = 0; i < typeB; ++i) {
        array.push_back(std::make_unique<ClassB>());
    }

    for (std::size_t i = array.size(); i < N; ++i) {
        array.push_back(std::make_unique<ClassC>());
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
