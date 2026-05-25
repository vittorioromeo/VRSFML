#pragma once

#include "SFML/Base/String.hpp"

#include <DoctestFwd.hpp>


namespace tst
{

template <>
struct StringMaker<sf::base::String>
{
    static sf::base::String convert(const sf::base::String& sv)
    {
        return sv;
    }
};

} // namespace tst
