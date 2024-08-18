#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/EnumClassBitwiseOps.hpp"

#include <cstdint>


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup window
/// \brief Enumeration of the window styles
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] Style : std::uint32_t
{
    None     = 0,      //!< No border / title bar (this flag and all others are mutually exclusive)
    Titlebar = 1 << 0, //!< Title bar + fixed border
    Resize   = 1 << 1, //!< Title bar + resizable border + maximize button
    Close    = 1 << 2, //!< Title bar + close button

    Default = Titlebar | Resize | Close //!< Default window style
};

SFML_BASE_DEFINE_ENUM_CLASS_BITWISE_OPS(Style);

////////////////////////////////////////////////////////////
/// \ingroup window
/// \brief Enumeration of the window states
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] State
{
    Windowed,  //!< Floating window
    Fullscreen //!< Fullscreen window
};

} // namespace sf
