#pragma once

#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct InputState
{
    sf::base::Array<sf::base::Optional<sf::Vec2i>, 32> fingerPositions{};

    sf::Vec2i mousePosition;

    sf::base::Array<bool, sf::Keyboard::KeyCount> keys{};
    sf::base::Array<bool, sf::Mouse::ButtonCount> buttons{};

    bool alt{};
    bool control{};
    bool shift{};
    bool system{};

    ////////////////////////////////////////////////////////////
    void apply(const sf::Event& event)
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
};
