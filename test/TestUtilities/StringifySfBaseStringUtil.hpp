#pragma once

#include "SFML/Base/String.hpp"

#include <DoctestFwd.hpp>


namespace doctest
{

template <>
struct StringMaker<sf::base::String>
{
    static doctest::String convert(const sf::base::String& sv)
    {
        return sv.data();
    }
};

} // namespace doctest
