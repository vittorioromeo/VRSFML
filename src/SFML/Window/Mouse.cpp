// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Mouse.hpp"

#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/WindowBase.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Assert.hpp"

#include <SDL3/SDL_mouse.h>


namespace sf
{
////////////////////////////////////////////////////////////
bool Mouse::isButtonPressed(Button button)
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
Vec2i Mouse::getPosition()
{
    return WindowContext::getSDLLayer().getGlobalMousePosition();
}


////////////////////////////////////////////////////////////
Vec2i Mouse::getPosition(const WindowBase& relativeTo)
{
    return getPosition() - relativeTo.getPosition();
}


////////////////////////////////////////////////////////////
void Mouse::setPosition(const Vec2i position)
{
    (void)WindowContext::getSDLLayer().setGlobalMousePosition(position);
}


////////////////////////////////////////////////////////////
void Mouse::setPosition(const Vec2i position, const WindowBase& relativeTo)
{
    setPosition(position + relativeTo.getPosition());
}

} // namespace sf
