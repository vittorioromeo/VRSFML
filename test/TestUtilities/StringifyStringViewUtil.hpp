#pragma once

#include "SFML/Base/StringView.hpp"

#include <DoctestFwd.hpp>

namespace doctest
{

template <>
struct StringMaker<sf::base::StringView>
{
    static doctest::String convert(const sf::base::StringView& sv)
    {
        return sv.data();
    }
};

} // namespace doctest
