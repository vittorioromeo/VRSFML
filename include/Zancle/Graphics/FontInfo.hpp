#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Export.hpp"

#include "Zancle/String/Utf8String.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief General information about a font
///
/// Returned by `za::Font::getInfo` and `za::FontFace::getInfo`.
/// Currently exposes the font family name; more fields may be
/// added in the future without breaking source compatibility.
///
////////////////////////////////////////////////////////////
struct ZA_GRAPHICS_API FontInfo
{
    Utf8String family; //!< Font family name (e.g. "DejaVu Sans"), UTF-8 encoded
};

} // namespace za
