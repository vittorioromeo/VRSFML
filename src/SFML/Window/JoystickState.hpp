#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/Joystick.hpp"

#include "SFML/Base/EnumArray.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Structure holding a joystick's state
///
////////////////////////////////////////////////////////////
struct JoystickState
{
    base::EnumArray<Joystick::Axis, float, Joystick::AxisCount> axes{}; //!< Position of each axis, in range [-100, 100]
    bool buttons[Joystick::ButtonCount]{};                              //!< Status of each button (true = pressed)
};

} // namespace sf::priv
