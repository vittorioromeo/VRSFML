#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/Touch.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
bool Touch::isDown(unsigned int finger)
{
    return priv::InputImpl::isTouchDown(finger);
}


////////////////////////////////////////////////////////////
Vector2i Touch::getPosition(unsigned int finger)
{
    return priv::InputImpl::getTouchPosition(finger);
}


////////////////////////////////////////////////////////////
Vector2i Touch::getPosition(unsigned int finger, const WindowBase& relativeTo)
{
    return priv::InputImpl::getTouchPosition(finger, relativeTo);
}

} // namespace sf
