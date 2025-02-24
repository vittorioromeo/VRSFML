#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/Mouse.hpp"

namespace sf::Mouse
{
////////////////////////////////////////////////////////////
bool isButtonPressed(Button button)
{
    return priv::InputImpl::isMouseButtonPressed(button);
}


////////////////////////////////////////////////////////////
Vector2i getPosition()
{
    return priv::InputImpl::getMousePosition();
}


////////////////////////////////////////////////////////////
Vector2i getPosition(const WindowBase& relativeTo)
{
    return priv::InputImpl::getMousePosition(relativeTo);
}


////////////////////////////////////////////////////////////
void setPosition(Vector2i position)
{
    priv::InputImpl::setMousePosition(position);
}


////////////////////////////////////////////////////////////
void setPosition(Vector2i position, const WindowBase& relativeTo)
{
    priv::InputImpl::setMousePosition(position, relativeTo);
}

} // namespace sf::Mouse
