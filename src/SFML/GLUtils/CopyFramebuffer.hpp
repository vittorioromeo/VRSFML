#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Copy framebuffer contents between specified FBOs
///
/// Binds the given source and destination framebuffer objects (FBOs) and performs
/// a blit operation between them with optional vertical flipping.
///
/// \param invertYAxis Whether to flip the source vertically during copy
/// \param size        Dimensions of the region to copy
/// \param srcFBO      Source framebuffer ID
/// \param dstFBO      Destination framebuffer ID
/// \param srcPos      Source region starting position (default: 0,0)
/// \param dstPos      Destination region starting position (default: 0,0)
///
////////////////////////////////////////////////////////////
void copyFramebuffer(bool         invertYAxis,
                     Vec2u        size,
                     unsigned int srcFBO,
                     unsigned int dstFBO,
                     Vec2u        srcPos = {0u, 0u},
                     Vec2u        dstPos = {0u, 0u});

////////////////////////////////////////////////////////////
/// \brief Copy framebuffer contents with vertical flipping
///
/// Copies source framebuffer contents to destination while vertically flipping
/// the image. Uses an intermediate framebuffer for OpenGL ES compatibility.
///
/// \param intermediateFBO        Reusable intermediate framebuffer ID
/// \param tmpTextureNativeHandle Temporary texture handle for intermediate operations
/// \param size                   Dimensions of the region to copy
/// \param srcFBO                 Source framebuffer ID
/// \param dstFBO                 Destination framebuffer ID
/// \param srcPos                 Source region starting position (default: 0,0)
/// \param dstPos                 Destination region starting position (default: 0,0)
///
/// \return True if copy succeeded, false otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool copyFlippedFramebufferViaIntermediateFBO(
    unsigned int intermediateFBO,
    unsigned int tmpTextureNativeHandle,
    Vec2u        size,
    unsigned int srcFBO,
    unsigned int dstFBO,
    Vec2u        srcPos = {0u, 0u},
    Vec2u        dstPos = {0u, 0u});

////////////////////////////////////////////////////////////
/// \brief Copy framebuffer contents with vertical flipping
///
/// Copies source framebuffer contents to destination while vertically flipping
/// the image.
///
/// \param size   Dimensions of the region to copy
/// \param srcFBO Source framebuffer ID
/// \param dstFBO Destination framebuffer ID
/// \param srcPos Source region starting position (default: 0,0)
/// \param dstPos Destination region starting position (default: 0,0)
///
////////////////////////////////////////////////////////////
[[nodiscard]] void copyFlippedFramebufferViaDirectBlit(
    Vec2u        size,
    unsigned int srcFBO,
    unsigned int dstFBO,
    Vec2u        srcPos = {0u, 0u},
    Vec2u        dstPos = {0u, 0u});

} // namespace sf::priv
