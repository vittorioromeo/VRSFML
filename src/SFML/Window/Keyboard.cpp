// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Keyboard.hpp"

#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Utf8String.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
bool Keyboard::isKeyPressed(const Key key)
{
    // TODO P0:
    // dispatch to OS-specific SFML impls

    return isKeyPressed(delocalize(key));
}


////////////////////////////////////////////////////////////
bool Keyboard::isKeyPressed(const Scancode code)
{
    // TODO P0:
    // dispatch to OS-specific SFML impls

    return WindowContext::getSDLLayer().isKeyPressedByScancode(code);
}


////////////////////////////////////////////////////////////
Keyboard::Key Keyboard::localize(const Scancode code)
{
    return WindowContext::getSDLLayer().localizeScancode(code);
}


////////////////////////////////////////////////////////////
Keyboard::Scancode Keyboard::delocalize(const Key key)
{
    return WindowContext::getSDLLayer().delocalizeScancode(key);
}


////////////////////////////////////////////////////////////
Utf8String Keyboard::getDescription(const Scancode code)
{
    return WindowContext::getSDLLayer().getScancodeDescription(code);
}


////////////////////////////////////////////////////////////
void Keyboard::setVirtualKeyboardVisible(const bool visible)
{
    WindowContext::getSDLLayer().setVirtualKeyboardVisible(visible);
}

} // namespace sf
