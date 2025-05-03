#pragma once

#include "Countdown.hpp"

#include "SFML/System/Vector2.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] HellPortal
{
    sf::Vector2f    position;
    Countdown       life;
    sf::base::SizeT catIdx;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Vector2f getDrawPosition() const
    {
        return position;
    }
};
