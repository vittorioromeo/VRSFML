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
/// \brief Structure holding a joystick's information
///
////////////////////////////////////////////////////////////
struct JoystickCapabilities
{
    unsigned int buttonCount{};                                        //!< Number of buttons supported by the joystick
    base::EnumArray<Joystick::Axis, bool, Joystick::AxisCount> axes{}; //!< Support for each axis
};


////////////////////////////////////////////////////////////
/// \brief Structure holding a joystick's state
///
////////////////////////////////////////////////////////////
struct JoystickState
{
    bool                                                        connected{}; //!< Is the joystick currently connected?
    base::EnumArray<Joystick::Axis, float, Joystick::AxisCount> axes{}; //!< Position of each axis, in range [-100, 100]
    bool buttons[Joystick::ButtonCount]{};                              //!< Status of each button (true = pressed)
};

} // namespace sf::priv


#if defined(SFML_SYSTEM_WINDOWS)

#include "SFML/Window/Win32/JoystickImpl.hpp"

#elif defined(SFML_SYSTEM_LINUX)

#include "SFML/Window/Unix/JoystickImpl.hpp"

#elif defined(SFML_SYSTEM_FREEBSD)

#include "SFML/Window/FreeBSD/JoystickImpl.hpp"

#elif defined(SFML_SYSTEM_OPENBSD) || defined(SFML_SYSTEM_IOS) || defined(SFML_SYSTEM_ANDROID)

#include "SFML/Window/Stub/StubJoystickImpl.hpp"
namespace sf::priv
{
using JoystickImpl = StubJoystickImpl;
}

#elif defined(SFML_SYSTEM_NETBSD)

#include "SFML/Window/NetBSD/JoystickImpl.hpp"

#elif defined(SFML_SYSTEM_MACOS)

#include "SFML/Window/macOS/JoystickImpl.hpp"

#elif defined(SFML_SYSTEM_EMSCRIPTEN)

#include "SFML/Window/Emscripten/JoystickImpl.hpp"

#endif
