#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/String.hpp"


namespace sf::Keyboard
{
////////////////////////////////////////////////////////////
bool isKeyPressed(Key key)
{
    return priv::InputImpl::isKeyPressed(key);
}


////////////////////////////////////////////////////////////
bool isKeyPressed(Scancode code)
{
    return priv::InputImpl::isKeyPressed(code);
}


////////////////////////////////////////////////////////////
Key localize(Scancode code)
{
    return priv::InputImpl::localize(code);
}


////////////////////////////////////////////////////////////
Scancode delocalize(Key key)
{
    return priv::InputImpl::delocalize(key);
}


////////////////////////////////////////////////////////////
String getDescription(Scancode code)
{
    return priv::InputImpl::getDescription(code);
}


////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(bool visible)
{
    priv::InputImpl::setVirtualKeyboardVisible(visible);
}

} // namespace sf::Keyboard
