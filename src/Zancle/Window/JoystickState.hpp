#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"

#include "Zancle/Window/Joystick.hpp"

#include "ZancleBase/EnumArray.hpp"


namespace za::priv
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

} // namespace za::priv
