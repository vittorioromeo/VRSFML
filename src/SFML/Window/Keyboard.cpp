#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/SDLLayer.hpp"

#include "SFML/System/String.hpp"


namespace sf::Keyboard
{
////////////////////////////////////////////////////////////
bool isKeyPressed(const Key key)
{
    // TODO P0:
    // dispatch to OS-specific SFML impls

    (void)priv::getSDLLayerSingleton(); // TODO P0: remove
    return isKeyPressed(delocalize(key));
}


////////////////////////////////////////////////////////////
bool isKeyPressed(const Scancode code)
{
    // TODO P0:
    // dispatch to OS-specific SFML impls

    const bool* keyboardState = SDL_GetKeyboardState(nullptr);
    SFML_BASE_ASSERT(keyboardState != nullptr);

    return keyboardState[priv::mapSFMLScancodeToSDL(code)];
}


////////////////////////////////////////////////////////////
Key localize(const Scancode code)
{
    return priv::localizeViaSDL(code);
}


////////////////////////////////////////////////////////////
Scancode delocalize(const Key key)
{
    return priv::delocalizeViaSDL(key);
}


////////////////////////////////////////////////////////////
String getDescription(const Scancode code)
{
    return SDL_GetKeyName(priv::mapSFMLKeycodeToSDL(localize(code)));
}


////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(const bool visible)
{
    // TODO P0:
    // not always applicable, dispatch to OS-specific SFML impls
}

} // namespace sf::Keyboard
