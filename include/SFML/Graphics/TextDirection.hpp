#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Enumeration of text direction options
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] TextDirection : unsigned char
{
    Unspecified, //!< Unspecified
    LeftToRight, //!< Left-to-right
    RightToLeft, //!< Right-to-left
    TopToBottom, //!< Top-to-bottom
    BottomToTop  //!< Bottom-to-top
};

} // namespace sf
