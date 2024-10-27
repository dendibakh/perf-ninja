#include "solution.h"

#include <random>
#include <array>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    std::array<int, 3> counter{0};
    for (size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        counter[value]++;
    }

    for (int i = 0; i < counter[0]; ++i) {
        array.push_back(std::make_unique<ClassA>());
    }
    for (int i = 0; i < counter[1]; ++i) {
        array.push_back(std::make_unique<ClassB>());
    }
    for (int i = 0; i < counter[2]; ++i) {
        array.push_back(std::make_unique<ClassC>());
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
