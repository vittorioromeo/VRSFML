#include "InputState.hpp"

#include "SFML/Window/Event.hpp"

#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
void InputState::apply(const sf::Event& event)
{
    if (const auto* e = event.getIf<sf::Event::KeyPressed>())
    {
        keys[static_cast<sf::base::SizeT>(e->code)] = true;

        alt     = e->alt;
        control = e->control;
        shift   = e->shift;
        system  = e->system;

        return;
    }

    if (const auto* e = event.getIf<sf::Event::KeyReleased>())
    {
        keys[static_cast<sf::base::SizeT>(e->code)] = false;

        alt     = e->alt;
        control = e->control;
        shift   = e->shift;
        system  = e->system;

        return;
    }

    if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>())
    {
        buttons[static_cast<sf::base::SizeT>(e->button)] = true;
        mousePosition                                    = e->position;

        return;
    }

    if (const auto* e = event.getIf<sf::Event::MouseButtonReleased>())
    {
        buttons[static_cast<sf::base::SizeT>(e->button)] = false;
        mousePosition                                    = e->position;

        return;
    }

    if (const auto* e = event.getIf<sf::Event::MouseMoved>())
    {
        mousePosition = e->position;

        return;
    }

    if (const auto* e = event.getIf<sf::Event::TouchBegan>())
    {
        fingerPositions[e->finger].emplace(e->position);

        return;
    }

    if (const auto* e = event.getIf<sf::Event::TouchMoved>())
    {
        fingerPositions[e->finger].emplace(e->position);

        return;
    }

    if (const auto* e = event.getIf<sf::Event::TouchEnded>())
    {
        fingerPositions[e->finger].reset();

        return;
    }
}
