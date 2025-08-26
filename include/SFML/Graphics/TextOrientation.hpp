#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Enumeration of text orientation options
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] TextOrientation : unsigned char
{
    Default,     //!< Default (left-to-right or right-to-left depending on detected script)
    TopToBottom, //!< Top-to-bottom
    BottomToTop  //!< Bottom-to-top
};

} // namespace sf
