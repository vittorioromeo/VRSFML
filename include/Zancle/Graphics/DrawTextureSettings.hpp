#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/Priv/TransformableMacros.hpp"

#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/RequireDesignatedInitializers.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Parameters for drawing a texture directly on a render target
///
/// `DrawTextureSettings` lets you stamp a `za::Texture` onto a
/// `za::RenderTarget` without needing to construct a full
/// `za::Sprite`. It bundles the standard transformable members
/// (`position`, `scale`, `origin`, `rotation`) with a sub-rectangle
/// and a tint color.
///
////////////////////////////////////////////////////////////
struct DrawTextureSettings
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    ZA_DEFINE_TRANSFORMABLE_DATA_MEMBERS;
    Rect2f textureRect{};       //!< Sub-rectangle of the source texture to display (`{}` means the full texture)
    Color  color{Color::White}; //!< Color used to tint (modulate) the texture
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::DrawTextureSettings
/// \ingroup graphics
///
/// `za::DrawTextureSettings` is the parameter pack for the
/// `za::RenderTarget::draw(const Texture&, const DrawTextureSettings&, ...)`
/// overload. It is the most convenient way to render a texture once
/// without keeping a `za::Sprite` around (e.g. for one-off blits).
///
/// \see `za::Sprite`, `za::RenderTarget`, `za::Texture`
///
////////////////////////////////////////////////////////////
