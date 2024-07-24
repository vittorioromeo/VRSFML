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
#include <SFML/Window/Stub/StubSensorImpl.hpp>

#include <SFML/System/Err.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
void StubSensorImpl::initialize()
{
    // err() << "Sensor API not implemented";
}


////////////////////////////////////////////////////////////
void StubSensorImpl::cleanup()
{
    // err() << "Sensor API not implemented";
}


////////////////////////////////////////////////////////////
bool StubSensorImpl::isAvailable(Sensor::Type /*sensor*/)
{
    // err() << "Sensor API not implemented";
    return false;
}


////////////////////////////////////////////////////////////
bool StubSensorImpl::open(Sensor::Type /*sensor*/)
{
    // err() << "Sensor API not implemented";
    return false;
}


////////////////////////////////////////////////////////////
void StubSensorImpl::close()
{
    // err() << "Sensor API not implemented";
}


////////////////////////////////////////////////////////////
Vector3f StubSensorImpl::update()
{
    // err() << "Sensor API not implemented";
    return {};
}


////////////////////////////////////////////////////////////
void StubSensorImpl::setEnabled(bool /*enabled*/)
{
    // err() << "Sensor API not implemented";
}

} // namespace sf::priv
