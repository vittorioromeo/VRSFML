// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Keyboard.hpp"

#include "Zancle/Window/SDLLayer.hpp"
#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/System/Utf8String.hpp"


namespace za
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

} // namespace za
