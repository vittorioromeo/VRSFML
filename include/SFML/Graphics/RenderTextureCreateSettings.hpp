#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextureWrapMode.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Settings used to create the framebuffer object backing a `sf::RenderTexture`
///
/// All members default to sensible values, so the struct can usually
/// be passed empty (e.g. via designated initializers) and only the
/// fields that need to differ from their defaults need to be set.
///
////////////////////////////////////////////////////////////
struct RenderTextureCreateSettings
{
    unsigned int    depthBits{0u};         //!< Number of bits for the depth buffer attachment (0 = no depth buffer)
    unsigned int    stencilBits{0u};       //!< Number of bits for the stencil buffer attachment (0 = no stencil buffer)
    unsigned int    antiAliasingLevel{0u}; //!< Multisampling (MSAA) sample count (0 or 1 = disabled)
    bool            sRgbCapable{false};    //!< Whether the texture should use sRGB encoding
    bool            smooth{false};         //!< Whether linear filtering should be enabled on the texture
    TextureWrapMode wrapMode{TextureWrapMode::Clamp}; //!< Wrap mode used when sampling the attached texture
};

} // namespace sf
