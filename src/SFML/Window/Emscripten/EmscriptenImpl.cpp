#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Event.hpp"
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/String.hpp"
#include "SFML/System/StringUtfUtils.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Optional.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <unordered_map>
#include <vector>

// TODO P0: focus seems not to work on mobile or unless alt tabbing
// - this is an an issue caused by the fact that imguiperwindowcontext's window doesn't have
//   focus on construction
// - it also seems that the focus cb in emscripten doesn't get triggered at the beginning


namespace sf::priv
{
////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(Keyboard::Key key)
{
    return false; // keyStatus[static_cast<base::SizeT>(key)];
}


////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(Keyboard::Scancode /* code */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
Keyboard::Key InputImpl::localize(Keyboard::Scancode /* code */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
Keyboard::Scancode InputImpl::delocalize(Keyboard::Key /* key */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
String InputImpl::getDescription(Keyboard::Scancode /* code */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
void InputImpl::setVirtualKeyboardVisible(bool /* visible */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool InputImpl::isMouseButtonPressed(Mouse::Button button)
{
    return false;
    // return mouseStatus[static_cast<base::SizeT>(button)];
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition()
{
    return {};
    // return mousePosition;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition(const WindowBase& /* relativeTo */)
{
    return getMousePosition();
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(Vector2i /* position */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(Vector2i position, const WindowBase& /* relativeTo */)
{
    setMousePosition(position);
}


////////////////////////////////////////////////////////////
bool InputImpl::isTouchDown(unsigned int finger)
{
    return false;
    // return touchStatus.find(finger) != touchStatus.end();
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int finger)
{
    return {};
    // const auto iter = touchStatus.find(finger);
    // return iter == touchStatus.end() ? Vector2i{} : iter->second;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int finger, const WindowBase& /* relativeTo */)
{
    return getTouchPosition(finger);
}

} // namespace sf::priv
