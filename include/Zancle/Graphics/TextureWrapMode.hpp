#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/IntTypes.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief How a texture handles coordinates outside the `[0, 1]` range
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] TextureWrapMode : base::U8
{
    Clamp,         //!< Clamp texture coordinates to the range `[0, 1]` (out-of-range samples reuse the edge texel)
    Repeat,        //!< Repeat the texture by wrapping coordinates (the integer part is discarded)
    MirroredRepeat //!< Repeat the texture, mirroring it when the integer part of the coordinate is odd
};

} // namespace za


////////////////////////////////////////////////////////////
/// \enum za::TextureWrapMode
/// \ingroup graphics
///
/// `za::TextureWrapMode` selects how a texture is sampled when
/// texture coordinates fall outside the canonical `[0, 1]` range.
/// This is the moral equivalent of OpenGL's `GL_TEXTURE_WRAP_*`
/// parameters.
///
/// \li `Clamp` -- the texture extends past its edges by reusing the
///     last row/column of texels. This is useful when you want to
///     avoid bleeding between repeats.
/// \li `Repeat` -- the texture tiles infinitely in both directions.
///     This is the natural choice for tileable patterns.
/// \li `MirroredRepeat` -- like `Repeat`, but every other tile is
///     mirrored, eliminating visible seams between repeats.
///
/// The wrap mode is set on a per-texture basis via
/// `za::Texture::setWrapMode` (see also `za::RenderTexture`).
///
/// \see `za::Texture`, `za::RenderTexture`
///
////////////////////////////////////////////////////////////
