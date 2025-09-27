#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/IntTypes.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] TextureWrapMode : base::U8
{
    Clamp,         //!< Clamp texture coordinates to the range `[0, 1]`
    Repeat,        //!< Repeat texture coordinates
    MirroredRepeat //!< Repeat texture coordinates, mirroring them when the integer part is odd
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \enum sf::TextureWrapMode
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Texture`, `sf::RenderTexture`
///
////////////////////////////////////////////////////////////
