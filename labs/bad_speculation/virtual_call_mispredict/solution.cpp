#include "solution.h"

#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);
    size_t class_cnts[3] = {};

    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        class_cnts[value]++;
    }

    for (std::size_t i = 0; i < N; i++) {
        if (class_cnts[0] != 0) {
            array.push_back(std::make_unique<ClassA>());
            class_cnts[0]--;
        } else if (class_cnts[1] != 0) {
            array.push_back(std::make_unique<ClassB>());
            class_cnts[1]--;
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
