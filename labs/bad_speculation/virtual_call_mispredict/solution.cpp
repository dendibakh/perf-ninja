#include "solution.h"

#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);
    InstanceArray arr1, arr2;
    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            array.push_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            arr1.push_back(std::make_unique<ClassB>());
        } else {
            arr2.push_back(std::make_unique<ClassC>());
        }
    }
    for (auto& x : arr1) {
        array.push_back(std::move(x));
    }
    for (auto& x : arr2) {
        array.push_back(std::move(x));
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
