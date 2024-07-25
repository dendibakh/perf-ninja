#include <cstdint>
#include <vector>
#include <memory>

// Assume this constant never changes
constexpr std::size_t N = 64 * 1024;

template<typename T>    constexpr uint8_t getIncrement() { return 1; }; 
template<>              constexpr uint8_t getIncrement<class ClassB>() { return 2; }
template<>              constexpr uint8_t getIncrement<class ClassC>() { return 3; } 

struct BaseClass {
    void handle(std::size_t& data) const { data += getIncrement<decltype(this)>(); };
};

struct ClassA : public BaseClass {};
struct ClassB : public BaseClass {};
struct ClassC : public BaseClass {};

using InstanceArray = std::vector<std::unique_ptr<BaseClass>>;

void generateObjects(InstanceArray& array);
void invoke(InstanceArray& array, std::size_t& data);
