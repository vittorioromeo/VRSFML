#pragma once

#include <doctest/parts/doctest_fwd.h>

#include <optional>

namespace doctest
{

template <typename T>
struct StringMaker<std::optional<T>>
{
    static doctest::String convert(const std::optional<T>&)
    {
        return ""; // TODO
    }
};

} // namespace doctest
