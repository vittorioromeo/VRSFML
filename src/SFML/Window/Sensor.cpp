#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Sensor.hpp>
#include <SFML/Window/SensorManager.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
bool Sensor::isAvailable(Type sensor)
{
    return priv::SensorManager::getInstance().isAvailable(sensor);
}


////////////////////////////////////////////////////////////
void Sensor::setEnabled(Type sensor, bool enabled)
{
    priv::SensorManager::getInstance().setEnabled(sensor, enabled);
}


////////////////////////////////////////////////////////////
Vector3f Sensor::getValue(Type sensor)
{
    return priv::SensorManager::getInstance().getValue(sensor);
}

} // namespace sf
