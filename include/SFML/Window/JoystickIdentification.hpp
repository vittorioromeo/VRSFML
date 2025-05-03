#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/System/String.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Structure holding a joystick's identification
///
////////////////////////////////////////////////////////////
struct SFML_WINDOW_API JoystickIdentification
{
    String       name{"No Joystick"}; //!< Name of the joystick
    unsigned int vendorId{};          //!< Manufacturer identifier
    unsigned int productId{};         //!< Product identifier
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
