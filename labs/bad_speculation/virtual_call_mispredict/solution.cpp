#include "solution.h"

#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);
    uint32_t cnt0 = 0, cnt1 = 0, cnt2 = 0;
    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0)
            cnt0++;
        else if (value == 1)
            cnt1++;
        else
            cnt2++;
    }
    for (auto i = 0; i < cnt0; i++)
        array.push_back(std::make_unique<ClassA>());
    for (auto i = 0; i < cnt1; i++)
        array.push_back(std::make_unique<ClassB>());
    for (auto i = 0; i < cnt2; i++)
        array.push_back(std::make_unique<ClassC>());
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
