// Header for SFML view-specific unit test stringification.

#pragma once

#include "GraphicsUtil.hpp"

#include "SFML/Graphics/View.hpp"


namespace sf
{
// Found by ADL when a `View::ScissorRect` operand needs rendering.
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const View::ScissorRect& scissorRect) noexcept;
} // namespace sf
