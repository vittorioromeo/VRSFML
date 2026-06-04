#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"

#include "Zancle/Window/Joystick.hpp"

#include "ZancleBase/EnumArray.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
/// \brief Structure holding a joystick's information
///
////////////////////////////////////////////////////////////
struct JoystickCapabilities
{
    unsigned int buttonCount{};                                        //!< Number of buttons supported by the joystick
    zb::EnumArray<Joystick::Axis, bool, Joystick::AxisCount> axes{}; //!< Support for each axis
};

} // namespace za::priv
