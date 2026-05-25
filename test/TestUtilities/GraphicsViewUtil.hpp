// Header for SFML view-specific unit test stringification.

#pragma once

#include "GraphicsUtil.hpp"

#include "SFML/Graphics/View.hpp"


namespace tst
{
template <>
struct StringMaker<sf::View::ScissorRect>
{
    static sf::base::String convert(const sf::View::ScissorRect& scissorRect);
};
} // namespace tst
