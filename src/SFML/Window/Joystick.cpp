#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Joystick.hpp"
#include "SFML/Window/JoystickImpl.hpp"
#include "SFML/Window/JoystickManager.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
bool Joystick::isConnected(unsigned int joystick)
{
    return priv::JoystickManager::getInstance().getState(joystick).connected;
}


////////////////////////////////////////////////////////////
unsigned int Joystick::getButtonCount(unsigned int joystick)
{
    return priv::JoystickManager::getInstance().getCapabilities(joystick).buttonCount;
}


////////////////////////////////////////////////////////////
bool Joystick::hasAxis(unsigned int joystick, Axis axis)
{
    return priv::JoystickManager::getInstance().getCapabilities(joystick).axes[axis];
}


////////////////////////////////////////////////////////////
bool Joystick::isButtonPressed(unsigned int joystick, unsigned int button)
{
    SFML_BASE_ASSERT(button < Joystick::ButtonCount && "Button must be less than Joystick::ButtonCount");
    return priv::JoystickManager::getInstance().getState(joystick).buttons[button];
}


////////////////////////////////////////////////////////////
float Joystick::getAxisPosition(unsigned int joystick, Axis axis)
{
    return priv::JoystickManager::getInstance().getState(joystick).axes[axis];
}


////////////////////////////////////////////////////////////
void Joystick::update()
{
    priv::JoystickManager::getInstance().update();
}

} // namespace sf
