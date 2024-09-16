#pragma once

#include "SFML/Base/StringView.hpp"

#include <doctest/parts/doctest_fwd.h>

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
