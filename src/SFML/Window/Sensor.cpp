#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Sensor.hpp"
#include "SFML/Window/SensorManager.hpp"
#include "SFML/Window/WindowContext.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
bool Sensor::isAvailable(Type sensor)
{
    return WindowContext::getSensorManager().isAvailable(sensor);
}


////////////////////////////////////////////////////////////
void Sensor::setEnabled(Type sensor, bool enabled)
{
    WindowContext::getSensorManager().setEnabled(sensor, enabled);
}


////////////////////////////////////////////////////////////
Vector3f Sensor::getValue(Type sensor)
{
    return WindowContext::getSensorManager().getValue(sensor);
}

} // namespace sf
