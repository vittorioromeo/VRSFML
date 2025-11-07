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
/// \brief Structure holding a joystick's identification
///
////////////////////////////////////////////////////////////
struct SFML_WINDOW_API JoystickIdentification
{
    UnicodeString name{"No Joystick"}; //!< Name of the joystick
    unsigned int  vendorId{};          //!< Manufacturer identifier
    unsigned int  productId{};         //!< Product identifier
};

} // namespace sf::priv


////////////////////////////////////////////////////////////
/// \namespace sf
/// \ingroup window
///
/// TODO P1: docs
///
/// \see sf::Joystick
///
////////////////////////////////////////////////////////////
