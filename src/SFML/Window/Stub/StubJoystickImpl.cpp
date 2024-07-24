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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/JoystickIdentification.hpp>
#include <SFML/Window/Stub/StubJoystickImpl.hpp>

#include <SFML/System/Err.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
void StubJoystickImpl::initialize()
{
    // err() << "Joystick API not implemented";
}


////////////////////////////////////////////////////////////
void StubJoystickImpl::cleanup()
{
    // err() << "Joystick API not implemented";
}


////////////////////////////////////////////////////////////
bool StubJoystickImpl::isConnected(unsigned int /* index */)
{
    // err() << "Joystick API not implemented";
    return false;
}


////////////////////////////////////////////////////////////
bool StubJoystickImpl::open(unsigned int /* index */)
{
    // err() << "Joystick API not implemented";
    return false;
}


////////////////////////////////////////////////////////////
void StubJoystickImpl::close()
{
    // err() << "Joystick API not implemented";
}


////////////////////////////////////////////////////////////
JoystickCaps StubJoystickImpl::getCapabilities() const
{
    // err() << "Joystick API not implemented";
    return {};
}


////////////////////////////////////////////////////////////
Joystick::Identification StubJoystickImpl::getIdentification() const
{
    // err() << "Joystick API not implemented";
    return {};
}


////////////////////////////////////////////////////////////
JoystickState StubJoystickImpl::update()
{
    // err() << "Joystick API not implemented";
    return {};
}

} // namespace sf::priv
