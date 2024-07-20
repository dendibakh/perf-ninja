#include "solution.h"

#include <random>

void generateObjects(InstanceArray &array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    InstanceArray arrays[3];
    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            arrays[0].push_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            arrays[1].push_back(std::make_unique<ClassB>());
        } else {
            arrays[2].push_back(std::make_unique<ClassC>());
        }
    }
    for (auto &v: arrays) {
        array.resize(array.size() + v.size());
        move(v.begin(), v.end(), array.end() - v.size());
    }
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray &array, std::size_t &data) {
    for (const auto &item: array) {
        item->handle(data);
    }
}
