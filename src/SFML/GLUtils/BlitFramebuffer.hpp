#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Blit a rectangular region between framebuffers with optional Y-axis inversion
///
/// This function copies a specified rectangular region from the read framebuffer
/// to the draw framebuffer, allowing vertical flipping through Y-axis inversion.
///
/// \param invertYAxis Whether to flip the source region vertically during copy
/// \param src         Source region coordinates and dimensions
/// \param dst         Destination region coordinates and dimensions
///
/// \note Uses `GL_NEAREST` filtering - appropriate for same-size copies.
///
////////////////////////////////////////////////////////////
void blitFramebuffer(bool invertYAxis, Rect2u src, Rect2u dst);

////////////////////////////////////////////////////////////
/// \brief Blit equal-sized regions between framebuffers with optional Y-axis inversion
///
/// This function copies a region of specified size from source position to destination
/// position from the read framebuffer to the draw framebuffers, allowing vertical flipping.
///
/// \param invertYAxis Whether to flip the source region vertically during copy
/// \param size        Dimensions of the region to copy
/// \param srcPos      Source region starting position
/// \param dstPos      Destination region starting position
///
/// \note Uses `GL_NEAREST` filtering - appropriate for same-size copies.
///
////////////////////////////////////////////////////////////
void blitFramebuffer(bool invertYAxis, Vec2u size, Vec2u srcPos, Vec2u dstPos);

} // namespace sf::priv
