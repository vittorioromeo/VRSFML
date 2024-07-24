#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Event.hpp>
#include <SFML/Window/EventUtils.hpp>


namespace sf::EventUtils
{
////////////////////////////////////////////////////////////
bool isClosedOrEscapeKeyPressed(const Event& event)
{
    return event.is<sf::Event::Closed>() ||
           (event.is<sf::Event::KeyPressed>() && event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape);
}

} // namespace sf::EventUtils
