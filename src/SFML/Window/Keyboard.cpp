#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/String.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
bool Keyboard::isKeyPressed(Key key)
{
    return priv::InputImpl::isKeyPressed(key);
}


////////////////////////////////////////////////////////////
bool Keyboard::isKeyPressed(Scancode code)
{
    return priv::InputImpl::isKeyPressed(code);
}


////////////////////////////////////////////////////////////
Keyboard::Key Keyboard::localize(Scancode code)
{
    return priv::InputImpl::localize(code);
}


////////////////////////////////////////////////////////////
Keyboard::Scancode Keyboard::delocalize(Key key)
{
    return priv::InputImpl::delocalize(key);
}


////////////////////////////////////////////////////////////
String Keyboard::getDescription(Scancode code)
{
    return priv::InputImpl::getDescription(code);
}


////////////////////////////////////////////////////////////
void Keyboard::setVirtualKeyboardVisible(bool visible)
{
    priv::InputImpl::setVirtualKeyboardVisible(visible);
}

} // namespace sf
