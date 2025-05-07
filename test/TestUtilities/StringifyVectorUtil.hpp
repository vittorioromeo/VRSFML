#pragma once

#include <DoctestFwd.hpp>

#include <vector>

namespace doctest
{

template <typename T>
struct StringMaker<std::vector<T>>
{
    static doctest::String convert(const std::vector<T>&)
    {
        return ""; // TODO P2:
    }
};

} // namespace doctest
