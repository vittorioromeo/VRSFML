#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/UnicodeString.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw text using a `GlyphMapping`
///
/// Unlike `TextData`, this struct does not include `characterSize`,
/// `outlineThickness`, or `bold` -- those are baked into the
/// `GlyphMapping` and taken from there.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API GlyphMappedTextData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;

    UnicodeString string;                     //!< String to display
    float         letterSpacing{1.f};         //!< Spacing factor between letters
    float         lineSpacing{1.f};           //!< Spacing factor between lines
    Color         fillColor{Color::White};    //!< Text fill color
    Color         outlineColor{Color::Black}; //!< Text outline color
    bool          italic{false};              //!< Italic characters
    bool          underlined{false};          //!< Underlined characters
    bool          strikeThrough{false};       //!< Strike through characters
};

} // namespace sf
