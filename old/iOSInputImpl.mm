#include <Zancle/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/InputImpl.hpp"
#include "Zancle/Window/VideoMode.hpp"
#include "Zancle/Window/Window.hpp"
#include "Zancle/Window/iOS/SFAppDelegate.hpp"

#include "Zancle/Err/Err.hpp"


namespace za::priv::InputImpl
{
////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Key /* key */)
{
    // Not applicable
    return false;
}


////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Scancode /* codes */)
{
    // Not applicable
    return false;
}


////////////////////////////////////////////////////////////
Keyboard::Key localize(Keyboard::Scancode /* code */)
{
    // Not applicable
    return Keyboard::Key::Unknown;
}


////////////////////////////////////////////////////////////
Keyboard::Scancode delocalize(Keyboard::Key /* key */)
{
    // Not applicable
    return Keyboard::Scan::Unknown;
}


////////////////////////////////////////////////////////////
String getDescription(Keyboard::Scancode /* code */)
{
    // Not applicable
    return "";
}


////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(bool visible)
{
    [[SFAppDelegate getInstance] setVirtualKeyboardVisible:visible];
}


////////////////////////////////////////////////////////////
bool isMouseButtonPressed(Mouse::Button /* button */)
{
    // Not applicable
    return false;
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition()
{
    return {};
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition(const WindowBase& /* relativeTo */)
{
    return getMousePosition();
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i /* position */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i /* position */, const WindowBase& /* relativeTo */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool isTouchDown(unsigned int finger)
{
    return [[SFAppDelegate getInstance] getTouchPosition:finger] != Vector2i(-1, -1);
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger)
{
    return [[SFAppDelegate getInstance] getTouchPosition:finger];
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger, const WindowBase& /* relativeTo */)
{
    return getTouchPosition(finger);
}

} // namespace za::priv::InputImpl
