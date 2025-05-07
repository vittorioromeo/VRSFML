#pragma once

#include "SFML/Base/Optional.hpp"

#include <DoctestFwd.hpp>

namespace doctest
{

template <typename T>
struct StringMaker<sf::base::Optional<T>>
{
    static doctest::String convert(const sf::base::Optional<T>&)
    {
        return ""; // TODO P3:
    }
};

} // namespace doctest
