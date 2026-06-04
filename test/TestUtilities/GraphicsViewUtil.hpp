// Header for SFML view-specific unit test stringification.

#pragma once

#include "GraphicsUtil.hpp"
#include "Zancle/Graphics/View.hpp"


namespace za
{
// Found by ADL when a `View::ScissorRect` operand needs rendering.
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const View::ScissorRect& scissorRect) noexcept;
} // namespace za
