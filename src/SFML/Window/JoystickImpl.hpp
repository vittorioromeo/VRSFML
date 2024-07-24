////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Config.hpp>

#include <SFML/Window/Joystick.hpp>

#include <SFML/Base/EnumArray.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Structure holding a joystick's information
///
////////////////////////////////////////////////////////////
struct JoystickCaps
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

#include <SFML/Window/Win32/JoystickImpl.hpp>

#elif defined(SFML_SYSTEM_LINUX)

#include <SFML/Window/Unix/JoystickImpl.hpp>

#elif defined(SFML_SYSTEM_FREEBSD)

#include <SFML/Window/FreeBSD/JoystickImpl.hpp>

#elif defined(SFML_SYSTEM_OPENBSD) || defined(SFML_SYSTEM_IOS) || defined(SFML_SYSTEM_ANDROID)

#include <SFML/Window/Stub/StubJoystickImpl.hpp>
namespace sf::priv
{
using JoystickImpl = StubJoystickImpl;
}

#elif defined(SFML_SYSTEM_NETBSD)

#include <SFML/Window/NetBSD/JoystickImpl.hpp>

#elif defined(SFML_SYSTEM_MACOS)

#include <SFML/Window/macOS/JoystickImpl.hpp>

#elif defined(SFML_SYSTEM_EMSCRIPTEN)

#include <SFML/Window/Emscripten/JoystickImpl.hpp>

#endif
