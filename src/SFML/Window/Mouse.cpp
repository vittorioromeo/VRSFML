#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/WindowBase.hpp"

#include <SDL3/SDL_mouse.h>


namespace sf::Mouse
{
////////////////////////////////////////////////////////////
bool isButtonPressed(Button button)
{
    const auto globalMouseState = SDL_GetGlobalMouseState(nullptr, nullptr);

    if (button == priv::getButtonFromSDLButton(SDL_BUTTON_LEFT))
        return (globalMouseState & SDL_BUTTON_LMASK) != 0;

    if (button == priv::getButtonFromSDLButton(SDL_BUTTON_MIDDLE))
        return (globalMouseState & SDL_BUTTON_MMASK) != 0;

    if (button == priv::getButtonFromSDLButton(SDL_BUTTON_RIGHT))
        return (globalMouseState & SDL_BUTTON_RMASK) != 0;

    if (button == priv::getButtonFromSDLButton(SDL_BUTTON_X1))
        return (globalMouseState & SDL_BUTTON_X1MASK) != 0;

    if (button == priv::getButtonFromSDLButton(SDL_BUTTON_X2))
        return (globalMouseState & SDL_BUTTON_X2MASK) != 0;

    SFML_BASE_ASSERT(false);
    return false;
}


////////////////////////////////////////////////////////////
Vector2i getPosition()
{
    Vector2f result;
    SDL_GetGlobalMouseState(&result.x, &result.y);
    return result.toVector2i();
}


////////////////////////////////////////////////////////////
Vector2i getPosition(const WindowBase& relativeTo)
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
    return getPosition(); // Calculation seems off with Emscripten
#else
    return getPosition() - relativeTo.getPosition();
#endif
}


////////////////////////////////////////////////////////////
void setPosition(Vector2i position)
{
    SDL_WarpMouseGlobal(static_cast<float>(position.x), static_cast<float>(position.y));
}


////////////////////////////////////////////////////////////
void setPosition(Vector2i position, const WindowBase& relativeTo)
{
    setPosition(position + relativeTo.getPosition());
}

} // namespace sf::Mouse
