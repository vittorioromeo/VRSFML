#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup graphics
/// \brief Integer type used to index into vertex arrays
///
/// Used by `sf::DrawIndexedVerticesSettings` and `sf::VertexBuffer`
/// when drawing indexed primitives. The 32-bit width allows
/// addressing very large vertex arrays without overflow.
///
////////////////////////////////////////////////////////////
using IndexType = unsigned int;

} // namespace sf
