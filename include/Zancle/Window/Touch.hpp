#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Export.hpp"

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Span.hpp"
#include "ZancleBase/StringView.hpp"


namespace za
{
class WindowBase;

////////////////////////////////////////////////////////////
/// \brief Give access to the real-time state of the touches
///
////////////////////////////////////////////////////////////
struct ZA_WINDOW_API Touch
{
    ////////////////////////////////////////////////////////////
    /// \brief Kind of physical touch input device
    ///
    /// Used to describe how to interpret the coordinates of
    /// touch events: a `Direct` device (e.g. a phone's touch
    /// screen) reports positions in window coordinates, while
    /// indirect devices (e.g. trackpads) report either device
    /// coordinates or screen-cursor-relative coordinates.
    ///
    ////////////////////////////////////////////////////////////
    enum class DeviceType
    {
        Direct,           //!< Touch screen with window-relative coordinates (e.g. phone or tablet display)
        IndirectAbsolute, //!< Trackpad reporting absolute device coordinates (independent of the screen cursor)
        IndirectRelative  //!< Trackpad reporting positions relative to the current screen cursor
    };

    ////////////////////////////////////////////////////////////
    /// \brief Description of one connected touch device
    ///
    ////////////////////////////////////////////////////////////
    struct Device // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        zb::U64        id;   //!< OS-assigned identifier of the touch device
        DeviceType       type; //!< Kind of touch device (direct / indirect)
        zb::StringView name; //!< Human-readable name of the touch device
    };

    ////////////////////////////////////////////////////////////
    /// \brief Enumerate the touch devices currently connected to the system
    ///
    /// The returned span is owned by the underlying SDL layer
    /// and remains valid until the next call to `getDevices()`
    /// or until the window context is destroyed.
    ///
    /// \return Span over the currently connected touch devices
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static zb::Span<Device> getDevices();
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::Touch
/// \ingroup window
///
/// `za::Touch` is the static interface for enumerating
/// physical touch input devices (touch screens, trackpads)
/// connected to the system. Per-finger position and pressure
/// data is delivered through the touch event subtypes
/// (`za::Event::TouchBegan`, `TouchMoved`, `TouchEnded`)
/// rather than queried directly here.
///
/// \see `za::Event::TouchBegan`, `za::Event::TouchMoved`, `za::Event::TouchEnded`,
///      `za::Joystick`, `za::Keyboard`, `za::Mouse`
///
////////////////////////////////////////////////////////////
