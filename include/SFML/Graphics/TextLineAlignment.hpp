#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Enumeration of text line alignment options
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] TextLineAlignment : unsigned char
{
    Default, //!< Automatically align lines by script direction, left-align left-to-right text and right-align right-to-left text
    Left,   //!< Force align all lines to the left, regardless of script direction
    Center, //!< Force align all lines centrally
    Right   //!< Force align lines to the right, regardless of script direction
};

} // namespace sf
