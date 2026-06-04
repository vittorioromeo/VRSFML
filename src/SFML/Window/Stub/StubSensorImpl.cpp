// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Stub/StubSensorImpl.hpp"

#include "SFML/System/Err.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
void StubSensorImpl::initialize()
{
    // errMsg("Sensor API not implemented");
}


////////////////////////////////////////////////////////////
void StubSensorImpl::cleanup()
{
    // errMsg("Sensor API not implemented");
}


////////////////////////////////////////////////////////////
bool StubSensorImpl::isAvailable(Sensor::Type /*sensor*/)
{
    // errMsg("Sensor API not implemented");
    return false;
}


////////////////////////////////////////////////////////////
bool StubSensorImpl::open(Sensor::Type /*sensor*/)
{
    // errMsg("Sensor API not implemented");
    return false;
}


////////////////////////////////////////////////////////////
void StubSensorImpl::close()
{
    // errMsg("Sensor API not implemented");
}


////////////////////////////////////////////////////////////
Vec3f StubSensorImpl::update()
{
    // errMsg("Sensor API not implemented");
    return {};
}


////////////////////////////////////////////////////////////
void StubSensorImpl::setEnabled(bool /*enabled*/)
{
    // errMsg("Sensor API not implemented");
}

} // namespace sf::priv
