// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Sensor.hpp"

#include "Zancle/Window/SensorManager.hpp"
#include "Zancle/Window/WindowContext.hpp"


namespace za
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
Vec3f Sensor::getValue(Type sensor)
{
    return WindowContext::getSensorManager().getValue(sensor);
}

} // namespace za
