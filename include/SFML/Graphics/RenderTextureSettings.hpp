#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Structure defining the settings of the Framebuffer
///        Object attached to a RenderTexture
///
////////////////////////////////////////////////////////////
struct RenderTextureSettings
{
    unsigned int depthBits{0};         //!< Bits of the depth buffer attachment
    unsigned int stencilBits{0};       //!< Bits of the stencil buffer attachment
    unsigned int antiAliasingLevel{0}; //!< Level of multisampling (MSAA)
    bool         sRgbCapable{false};   //!< Whether the texture should use sRGB encoding
};

} // namespace sf
