#include "solution.h"

#include <array>
#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    std::array<int, 3> cnts{};
    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        ++cnts[value];
    }

    for (std::size_t i = 0; i < cnts[0]; i++) {
        array.push_back(std::make_unique<ClassA>());
    }
    for (std::size_t i = 0; i < cnts[1]; i++) {
        array.push_back(std::make_unique<ClassB>());
    }
    for (std::size_t i = 0; i < cnts[2]; i++) {
        array.push_back(std::make_unique<ClassC>());
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
