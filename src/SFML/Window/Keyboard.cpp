// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Keyboard.hpp"

#include "SFML/Window/SDLLayer.hpp"

#include "SFML/System/UnicodeString.hpp"


namespace sf::Keyboard
{
////////////////////////////////////////////////////////////
bool isKeyPressed(const Key key)
{
    // TODO P0:
    // dispatch to OS-specific SFML impls

    return isKeyPressed(delocalize(key));
}


////////////////////////////////////////////////////////////
bool isKeyPressed(const Scancode code)
{
    // TODO P0:
    // dispatch to OS-specific SFML impls

    return priv::getSDLLayerSingleton().isKeyPressedByScancode(code);
}


////////////////////////////////////////////////////////////
Key localize(const Scancode code)
{
    return priv::getSDLLayerSingleton().localizeScancode(code);
}


////////////////////////////////////////////////////////////
Scancode delocalize(const Key key)
{
    return priv::getSDLLayerSingleton().delocalizeScancode(key);
}


////////////////////////////////////////////////////////////
UnicodeString getDescription(const Scancode code)
{
    return priv::getSDLLayerSingleton().getScancodeDescription(code);
}


////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(const bool visible)
{
    // TODO P0:
    // not always applicable, dispatch to OS-specific SFML impls
}

} // namespace sf::Keyboard
