#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/SDLLayer.hpp"

#include "SFML/System/String.hpp"
#include "SFML/System/StringUtfUtils.hpp"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>

#include <emscripten.h>
#include <emscripten/html5.h>

// TODO P0: focus seems not to work on mobile or unless alt tabbing
// - this is an an issue caused by the fact that imguiperwindowcontext's window doesn't have
//   focus on construction
// - it also seems that the focus cb in emscripten doesn't get triggered at the beginning


namespace sf::priv
{
////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(const Keyboard::Key key)
{
    (void)getSDLLayerSingleton();
    return InputImpl::isKeyPressed(InputImpl::delocalize(key));
}


////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(const Keyboard::Scancode code)
{
    const bool* keyboardState = SDL_GetKeyboardState(nullptr);
    SFML_BASE_ASSERT(keyboardState != nullptr);

    return keyboardState[mapSFMLScancodeToSDL(code)];
}


////////////////////////////////////////////////////////////
Keyboard::Key InputImpl::localize(const Keyboard::Scancode code)
{
    return priv::localizeViaSDL(code);
}


////////////////////////////////////////////////////////////
Keyboard::Scancode InputImpl::delocalize(const Keyboard::Key key)
{
    return priv::delocalizeViaSDL(key);
}


////////////////////////////////////////////////////////////
String InputImpl::getDescription(const Keyboard::Scancode code)
{
    return SDL_GetScancodeName(mapSFMLScancodeToSDL(code));
}


////////////////////////////////////////////////////////////
void InputImpl::setVirtualKeyboardVisible(bool /* visible */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool InputImpl::isMouseButtonPressed(Mouse::Button button)
{
    const auto globalMouseState = SDL_GetGlobalMouseState(nullptr, nullptr);

    if (button == getButtonFromSDLButton(SDL_BUTTON_LEFT))
        return (globalMouseState & SDL_BUTTON_LMASK) != 0;

    if (button == getButtonFromSDLButton(SDL_BUTTON_MIDDLE))
        return (globalMouseState & SDL_BUTTON_MMASK) != 0;

    if (button == getButtonFromSDLButton(SDL_BUTTON_RIGHT))
        return (globalMouseState & SDL_BUTTON_RMASK) != 0;

    if (button == getButtonFromSDLButton(SDL_BUTTON_X1))
        return (globalMouseState & SDL_BUTTON_X1MASK) != 0;

    if (button == getButtonFromSDLButton(SDL_BUTTON_X2))
        return (globalMouseState & SDL_BUTTON_X2MASK) != 0;

    SFML_BASE_ASSERT(false);
    return false;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition()
{
    Vector2f result;
    SDL_GetGlobalMouseState(&result.x, &result.y);
    return result.toVector2i();
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition(const WindowBase& /* relativeTo */)
{
    Vector2f result;
    SDL_GetMouseState(&result.x, &result.y);
    return result.toVector2i();
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(const Vector2i position)
{
    SDL_WarpMouseGlobal(static_cast<float>(position.x), static_cast<float>(position.y));
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(Vector2i position, const WindowBase& /* relativeTo */)
{
    setMousePosition(position);
}


////////////////////////////////////////////////////////////
bool InputImpl::isTouchDown(unsigned int finger)
{
    // TODO P0:
    return false;
    // return touchStatus.find(finger) != touchStatus.end();
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int finger)
{
    // TODO P0:
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
