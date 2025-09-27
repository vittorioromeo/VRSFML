// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/EventUtils.hpp"

#include "SFML/Window/Event.hpp"


namespace sf::EventUtils
{
////////////////////////////////////////////////////////////
bool isClosedOrEscapeKeyPressed(const Event& event)
{
    return event.is<sf::Event::Closed>() ||
           (event.is<sf::Event::KeyPressed>() && event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape);
}

} // namespace sf::EventUtils
