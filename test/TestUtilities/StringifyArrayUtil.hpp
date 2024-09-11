#pragma once

#include "SFML/Base/SizeT.hpp"

#include <doctest/parts/doctest_fwd.h>

#include <array>

namespace doctest
{

template <typename T, sf::base::SizeT N>
struct StringMaker<std::array<T, N>>
{
    static doctest::String convert(const std::array<T, N>&)
    {
        return ""; // TODO P2:
    }
};

} // namespace doctest
