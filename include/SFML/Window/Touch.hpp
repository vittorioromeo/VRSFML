#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Span.hpp"
#include "SFML/Base/StringView.hpp"


namespace sf
{
class WindowBase;

////////////////////////////////////////////////////////////
/// \brief Give access to the real-time state of the touches
///
////////////////////////////////////////////////////////////
struct SFML_WINDOW_API Touch
{
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    enum class DeviceType
    {
        Direct,           //!< Touch screen with window-relative coordinates
        IndirectAbsolute, //!< Trackpad with absolute device coordinates
        IndirectRelative  //!< Trackpad with screen cursor-relative coordinates
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct Device // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        base::U64        id;   //!< Id of the touch device
        DeviceType       type; //!< Type of the touch device
        base::StringView name; //!< Name of the touch device
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Span<Device> getDevices();
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::Touch
/// \ingroup window
///
/// TODO P1: docs
///
/// \see `sf::Joystick`, `sf::Keyboard`, `sf::Mouse`
///
////////////////////////////////////////////////////////////
