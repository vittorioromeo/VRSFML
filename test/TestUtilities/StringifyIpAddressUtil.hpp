#pragma once

#include "SFML/Network/IpAddress.hpp"

#include "SFML/Base/String.hpp"

#include <DoctestFwd.hpp>

namespace tst
{

template <>
struct StringMaker<sf::IpAddress>
{
    static sf::base::String convert(const sf::IpAddress&)
    {
        return detail::defaultStringification();
    }
};

} // namespace tst
