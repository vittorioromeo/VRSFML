#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Image;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API TextureAtlasUtils
{
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Rect2f> add(
        Texture&        targetTexture,
        RectPacker&     rectPacker,
        Vec2u           padding,
        const base::U8* pixels,
        Vec2u           size);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Rect2f> add(Texture& targetTexture, RectPacker& rectPacker, Vec2u padding, const Image& image);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Rect2f> add(Texture&       targetTexture,
                                                    RectPacker&    rectPacker,
                                                    Vec2u          padding,
                                                    const Texture& texture);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TextureAtlasUtils
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see sf::Texture, sf::RectPacker
///
////////////////////////////////////////////////////////////
