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
/// \brief Data required to draw text using a `GlyphMapping`
///
/// Unlike `TextData`, this struct does not include `characterSize`,
/// `outlineThickness`, or `bold` -- those are baked into the
/// `GlyphMapping` and taken from there.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] ZA_GRAPHICS_API GlyphMappedTextData
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;

    Utf8String string;                     //!< UTF-8 string to display
    float      letterSpacing{1.f};         //!< Spacing factor between letters
    float      lineSpacing{1.f};           //!< Spacing factor between lines
    Color      fillColor{Color::White};    //!< Text fill color
    Color      outlineColor{Color::Black}; //!< Text outline color
    bool       italic{false};              //!< Italic characters
    bool       underlined{false};          //!< Underlined characters
    bool       strikeThrough{false};       //!< Strike through characters
};

} // namespace za
