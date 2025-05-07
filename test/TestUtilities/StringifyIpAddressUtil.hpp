#pragma once

#include "SFML/Network/IpAddress.hpp"

#include <DoctestFwd.hpp>

namespace doctest
{

template <>
struct StringMaker<sf::IpAddress>
{
    static doctest::String convert(const sf::IpAddress&)
    {
        return ""; // TODO P3:
    }
};

} // namespace doctest
