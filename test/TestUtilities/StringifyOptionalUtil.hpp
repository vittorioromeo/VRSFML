#pragma once

#include <doctest/parts/doctest_fwd.h>

#include <SFML/System/Optional.hpp>

namespace doctest
{

template <typename T>
struct StringMaker<sf::Optional<T>>
{
    static doctest::String convert(const sf::Optional<T>&)
    {
        return ""; // TODO
    }
};

} // namespace doctest
