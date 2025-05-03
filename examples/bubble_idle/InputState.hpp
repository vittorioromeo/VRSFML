#pragma once

#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct InputState
{
    sf::base::Array<sf::base::Optional<sf::Vector2i>, 32> fingerPositions{};

    sf::Vector2i mousePosition;

    sf::base::Array<bool, sf::Keyboard::KeyCount> keys{};
    sf::base::Array<bool, sf::Mouse::ButtonCount> buttons{};

    bool alt{};
    bool control{};
    bool shift{};
    bool system{};

    ////////////////////////////////////////////////////////////
    void apply(const sf::Event& event)
    {
        event.match(
            [&](const sf::Event::KeyPressed& e)
        {
            keys[static_cast<sf::base::SizeT>(e.code)] = true;

            alt     = e.alt;
            control = e.control;
            shift   = e.shift;
            system  = e.system;
        },
            [&](const sf::Event::KeyReleased& e)
        {
            keys[static_cast<sf::base::SizeT>(e.code)] = false;

            alt     = e.alt;
            control = e.control;
            shift   = e.shift;
            system  = e.system;
        },
            [&](const sf::Event::MouseButtonPressed& e)
        {
            buttons[static_cast<sf::base::SizeT>(e.button)] = true;
            mousePosition                                   = e.position;
        },
            [&](const sf::Event::MouseButtonReleased& e)
        {
            buttons[static_cast<sf::base::SizeT>(e.button)] = false;
            mousePosition                                   = e.position;
        },
            [&](const sf::Event::MouseMoved& e) { mousePosition = e.position; },
            [&](const sf::Event::TouchBegan& e) { fingerPositions[e.finger].emplace(e.position); },
            [&](const sf::Event::TouchMoved& e) { fingerPositions[e.finger].emplace(e.position); },
            [&](const sf::Event::TouchEnded& e) { fingerPositions[e.finger].reset(); },
            [](const auto&) {});
    }
};
