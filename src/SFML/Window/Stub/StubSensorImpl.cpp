#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
Vec3f StubSensorImpl::update()
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
