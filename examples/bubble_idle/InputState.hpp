#pragma once

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
class Event;
}


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
    void apply(const sf::Event& event);
};
