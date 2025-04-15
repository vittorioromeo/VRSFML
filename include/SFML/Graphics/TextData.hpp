#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/String.hpp"

#include "SFML/Base/IntTypes.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Enumeration of text drawing styles
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] TextStyle : base::U8
{
    Regular       = 0,      //!< Regular characters, no style
    Bold          = 1 << 0, //!< Bold characters
    Italic        = 1 << 1, //!< Italic characters
    Underlined    = 1 << 2, //!< Underlined characters
    StrikeThrough = 1 << 3  //!< Strike through characters
};

////////////////////////////////////////////////////////////
/// \brief Data required to draw a rounded rectangle shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API TextData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;

    String       string{};                   //!< String to display
    unsigned int characterSize{30u};         //!< Base size of characters, in pixels
    float        letterSpacing{1.f};         //!< Spacing factor between letters
    float        lineSpacing{1.f};           //!< Spacing factor between lines
    Color        fillColor{Color::White};    //!< Text fill color
    Color        outlineColor{Color::Black}; //!< Text outline color
    float        outlineThickness{0.f};      //!< Thickness of the text's outline
    TextStyle    style{TextStyle::Regular};  //!< Text style (see Style enum)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RoundedRectangleShapeData
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shape`
///
////////////////////////////////////////////////////////////
