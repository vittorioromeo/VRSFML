#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/System/String.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Holds various information about a font
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API FontInfo
{
    unsigned int id{};                 //!< A unique ID that identifies the font
    String       family;               //!< The font family
    bool         hasKerning{};         //!< Has kerning information
    bool         hasVerticalMetrics{}; //!< Has native vertical metrics
};

} // namespace sf
