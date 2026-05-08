// Header for SFML view-specific unit test stringification.

#pragma once

#include "GraphicsUtil.hpp"

#include "SFML/Graphics/View.hpp"


namespace doctest
{
template <>
struct StringMaker<sf::View::ScissorRect>
{
    static String convert(const sf::View::ScissorRect& scissorRect);
};
} // namespace doctest
