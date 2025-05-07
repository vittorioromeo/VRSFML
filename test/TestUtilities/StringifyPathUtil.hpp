#pragma once

#include "SFML/System/Path.hpp"

#include <DoctestFwd.hpp>

namespace doctest
{

template <>
struct StringMaker<sf::Path>
{
    static doctest::String convert(const sf::Path&)
    {
        return ""; // TODO P2:
    }
};

} // namespace doctest
