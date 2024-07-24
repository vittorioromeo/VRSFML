#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/InputImpl.hpp>
#include <SFML/Window/Mouse.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
bool Mouse::isButtonPressed(Button button)
{
    return priv::InputImpl::isMouseButtonPressed(button);
}


////////////////////////////////////////////////////////////
Vector2i Mouse::getPosition()
{
    return priv::InputImpl::getMousePosition();
}


////////////////////////////////////////////////////////////
Vector2i Mouse::getPosition(const WindowBase& relativeTo)
{
    return priv::InputImpl::getMousePosition(relativeTo);
}


////////////////////////////////////////////////////////////
void Mouse::setPosition(Vector2i position)
{
    priv::InputImpl::setMousePosition(position);
}


////////////////////////////////////////////////////////////
void Mouse::setPosition(Vector2i position, const WindowBase& relativeTo)
{
    priv::InputImpl::setMousePosition(position, relativeTo);
}

} // namespace sf
