#include "solution.h"

#include <random>
#include <algorithm>

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

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    std::sort(
        begin(array)
        , end(array)
        , [](const std::unique_ptr<BaseClass> &a, const std::unique_ptr<BaseClass> &b) {
            return a->classType < b->classType;
        });

    for (const auto& item: array) {
        item->handle(data);
    }
}
