#include <cstdint>
#include <vector>
#include <memory>

#define SOLUTION

// Assume this constant never changes
constexpr std::size_t N = 64 * 1024;

struct BaseClass
{
    virtual ~BaseClass() = default;

    virtual void handle(std::size_t &data) const = 0;

#ifdef SOLUTION
    virtual int type() const = 0;
#endif
};

struct ClassA : public BaseClass
{
    void handle(std::size_t &data) const override
    {
        data += 1;
    }

#ifdef SOLUTION
    int type() const override
    {
        return 0;
    }
#endif
};

struct ClassB : public BaseClass
{
    void handle(std::size_t &data) const override
    {
        data += 2;
    }

#ifdef SOLUTION
    int type() const override
    {
        return 1;
    }
#endif
};

struct ClassC : public BaseClass
{
    void handle(std::size_t &data) const override
    {
        data += 3;
    }

#ifdef SOLUTION
    int type() const override
    {
        return 2;
    }
#endif
};

using InstanceArray = std::vector<std::unique_ptr<BaseClass>>;

void generateObjects(InstanceArray &array);
void invoke(InstanceArray &array, std::size_t &data);
