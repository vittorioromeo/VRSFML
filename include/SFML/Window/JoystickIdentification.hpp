#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/System/UnicodeString.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Stable identification data for a connected joystick
///
/// `JoystickIdentification` bundles together the human
/// readable name of a joystick and the USB-style vendor /
/// product identifiers reported by the OS. It is used
/// internally by `sf::priv::JoystickManager` and exposed to
/// user code through `sf::Joystick::Query::getName`,
/// `getVendorId`, and `getProductId`.
///
/// \see `sf::Joystick`
///
////////////////////////////////////////////////////////////
struct SFML_WINDOW_API JoystickIdentification
{
    UnicodeString name{
        "No Joystick"}; //!< Human-readable name of the joystick (defaults to a placeholder when no joystick is connected)
    unsigned int vendorId{};  //!< USB vendor identifier reported by the OS
    unsigned int productId{}; //!< USB product identifier reported by the OS
};

} // namespace sf::priv
