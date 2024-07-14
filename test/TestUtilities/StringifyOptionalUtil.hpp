#pragma once

#include <doctest/parts/doctest_fwd.h>

#include <SFML/Base/Optional.hpp>

namespace doctest
{

template <typename T>
struct StringMaker<sf::base::Optional<T>>
{
    static doctest::String convert(const sf::base::Optional<T>&)
    {
        return ""; // TODO
    }
};

} // namespace doctest
