#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Parameters for drawing a texture directly on a render target
///
/// `DrawTextureSettings` lets you stamp a `sf::Texture` onto a
/// `sf::RenderTarget` without needing to construct a full
/// `sf::Sprite`. It bundles the standard transformable members
/// (`position`, `scale`, `origin`, `rotation`) with a sub-rectangle
/// and a tint color.
///
////////////////////////////////////////////////////////////
struct DrawTextureSettings
{
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;
    Rect2f textureRect{};       //!< Sub-rectangle of the source texture to display (`{}` means the full texture)
    Color  color{Color::White}; //!< Color used to tint (modulate) the texture
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::DrawTextureSettings
/// \ingroup graphics
///
/// `sf::DrawTextureSettings` is the parameter pack for the
/// `sf::RenderTarget::draw(const Texture&, const DrawTextureSettings&, ...)`
/// overload. It is the most convenient way to render a texture once
/// without keeping a `sf::Sprite` around (e.g. for one-off blits).
///
/// \see `sf::Sprite`, `sf::RenderTarget`, `sf::Texture`
///
////////////////////////////////////////////////////////////
