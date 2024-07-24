#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Export.hpp>

#include <SFML/System/Rect.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Structure describing a glyph
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API Glyph
{
    float     advance{};   //!< Offset to move horizontally to the next character
    int       lsbDelta{};  //!< Left offset after forced autohint. Internally used by getKerning()
    int       rsbDelta{};  //!< Right offset after forced autohint. Internally used by getKerning()
    FloatRect bounds;      //!< Bounding rectangle of the glyph, in coordinates relative to the baseline
    IntRect   textureRect; //!< Texture coordinates of the glyph inside the font's texture
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::Glyph
/// \ingroup graphics
///
/// A glyph is the visual representation of a character.
///
/// The sf::Glyph structure provides the information needed
/// to handle the glyph:
/// \li its coordinates in the font's texture
/// \li its bounding rectangle
/// \li the offset to apply to get the starting position of the next glyph
///
/// \see sf::Font
///
////////////////////////////////////////////////////////////
