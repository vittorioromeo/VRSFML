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
bool Sensor::isAvailable(WindowContext& windowContext, Type sensor)
{
    return windowContext.getSensorManager().isAvailable(sensor);
}


////////////////////////////////////////////////////////////
void Sensor::setEnabled(WindowContext& windowContext, Type sensor, bool enabled)
{
    windowContext.getSensorManager().setEnabled(sensor, enabled);
}


////////////////////////////////////////////////////////////
Vector3f Sensor::getValue(WindowContext& windowContext, Type sensor)
{
    return windowContext.getSensorManager().getValue(sensor);
}

} // namespace sf
