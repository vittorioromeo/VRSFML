#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Sensor.hpp"
#include "SFML/Window/SensorManager.hpp"
#include "SFML/Window/WindowContext.hpp"


namespace sf::Sensor
{
////////////////////////////////////////////////////////////
bool isAvailable(Type sensor)
{
    return WindowContext::ensureInstalled().getSensorManager().isAvailable(sensor);
}


////////////////////////////////////////////////////////////
void setEnabled(Type sensor, bool enabled)
{
    WindowContext::ensureInstalled().getSensorManager().setEnabled(sensor, enabled);
}


////////////////////////////////////////////////////////////
Vector3f getValue(Type sensor)
{
    return WindowContext::ensureInstalled().getSensorManager().getValue(sensor);
}

} // namespace sf::Sensor
