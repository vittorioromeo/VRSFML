#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/System/UnicodeString.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief General information about a font
///
/// Returned by `sf::Font::getInfo` and `sf::FontFace::getInfo`.
/// Currently exposes the font family name; more fields may be
/// added in the future without breaking source compatibility.
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API FontInfo
{
    UnicodeString family; //!< Font family name (e.g. "DejaVu Sans")
};

} // namespace sf
