#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/JoystickIdentification.hpp"
#include "SFML/Window/JoystickManager.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Joystick::Identification Joystick::getIdentification(unsigned int joystick)
{
    return priv::JoystickManager::getInstance().getIdentification(joystick);
}


} // namespace sf
