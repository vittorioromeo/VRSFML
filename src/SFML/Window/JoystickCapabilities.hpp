#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/Joystick.hpp"

#include "SFML/Base/EnumArray.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Structure holding a joystick's information
///
////////////////////////////////////////////////////////////
struct JoystickCapabilities
{
    unsigned int buttonCount{};                                        //!< Number of buttons supported by the joystick
    base::EnumArray<Joystick::Axis, bool, Joystick::AxisCount> axes{}; //!< Support for each axis
};

} // namespace sf::priv
