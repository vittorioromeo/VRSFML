#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/System/Rect.hpp"

#include "SFML/Base/IntTypes.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Structure describing a glyph
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API Glyph
{
    float     advance{};   //!< Offset to move horizontally to the next character
    FloatRect bounds;      //!< Bounding rectangle of the glyph, in coordinates relative to the baseline
    FloatRect textureRect; //!< Texture coordinates of the glyph inside the font's texture
    base::I16 lsbDelta{};  //!< Left offset after forced autohint. Internally used by `getKerning()`
    base::I16 rsbDelta{};  //!< Right offset after forced autohint. Internally used by `getKerning()`
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::Glyph
/// \ingroup graphics
///
/// A glyph is the visual representation of a character.
///
/// The `sf::Glyph` structure provides the information needed
/// to handle the glyph:
/// \li its coordinates in the font's texture
/// \li its bounding rectangle
/// \li the offset to apply to get the starting position of the next glyph
///
/// \see `sf::Font`
///
////////////////////////////////////////////////////////////
