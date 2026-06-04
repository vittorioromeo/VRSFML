#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/Export.hpp"
#include "Zancle/Graphics/Priv/TransformableMacros.hpp"
#include "Zancle/System/Utf8String.hpp"
#include "ZancleBase/RequireDesignatedInitializers.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \struct za::TextData
/// \ingroup graphics
///
/// \brief Data required to draw text
///
/// `za::TextData` is a simple struct that aggregates all the visual
/// and transformational properties of a piece of text. This includes
/// the text string itself, character size, styling (like bold or italic),
/// colors, spacing, and transformation attributes (position, rotation,
/// scale, origin).
///
/// Its POD nature makes it suitable for scenarios like batched rendering,
/// where an array of `TextData` might be processed, or for serializing
/// text properties to and from files.
///
/// \see za::Text, za::Color, za::Utf8String
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] ZA_GRAPHICS_API TextData
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;

    Utf8String   string;                     //!< UTF-8 string to display
    unsigned int characterSize;              //!< Base size of characters, in pixels
    float        letterSpacing{1.f};         //!< Spacing factor between letters
    float        lineSpacing{1.f};           //!< Spacing factor between lines
    Color        fillColor{Color::White};    //!< Text fill color
    Color        outlineColor{Color::Black}; //!< Text outline color
    float        outlineThickness{0.f};      //!< Thickness of the text's outline
    bool         bold{false};                //!< Bold characters
    bool         italic{false};              //!< Italic characters
    bool         underlined{false};          //!< Underlined characters
    bool         strikeThrough{false};       //!< Strike through characters
};

} // namespace za
