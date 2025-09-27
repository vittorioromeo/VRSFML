// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
Vec2i getPosition()
{
    return priv::getSDLLayerSingleton().getGlobalMousePosition();
}


////////////////////////////////////////////////////////////
Vec2i getPosition(const WindowBase& relativeTo)
{
    return getPosition() - relativeTo.getPosition();
}


////////////////////////////////////////////////////////////
void setPosition(const Vec2i position)
{
    (void)priv::getSDLLayerSingleton().setGlobalMousePosition(position);
}


////////////////////////////////////////////////////////////
void setPosition(const Vec2i position, const WindowBase& relativeTo)
{
    setPosition(position + relativeTo.getPosition());
}

} // namespace sf::Mouse
