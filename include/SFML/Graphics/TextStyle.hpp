#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Enumeration of text drawing styles
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] TextStyle : unsigned char
{
    Regular       = 0,      //!< Regular characters, no style
    Bold          = 1 << 0, //!< Bold characters
    Italic        = 1 << 1, //!< Italic characters
    Underlined    = 1 << 2, //!< Underlined characters
    StrikeThrough = 1 << 3  //!< Strike through characters
};

} // namespace sf
