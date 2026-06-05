#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Export.hpp"

#include "Zancle/String/Utf8String.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
/// \brief Stable identification data for a connected joystick
///
/// `JoystickIdentification` bundles together the human
/// readable name of a joystick and the USB-style vendor /
/// product identifiers reported by the OS. It is used
/// internally by `za::priv::JoystickManager` and exposed to
/// user code through `za::Joystick::Query::getName`,
/// `getVendorId`, and `getProductId`.
///
/// \see `za::Joystick`
///
////////////////////////////////////////////////////////////
struct ZA_WINDOW_API JoystickIdentification
{
    Utf8String   name{"No Joystick"}; //!< Human-readable name (defaults to placeholder when no joystick connected)
    unsigned int vendorId{};          //!< USB vendor identifier reported by the OS
    unsigned int productId{};         //!< USB product identifier reported by the OS
};

} // namespace za::priv
