#pragma once

#include "Countdown.hpp"

#include "SFML/System/Vec2.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] HellPortal
{
    sf::Vec2f       position;
    Countdown       life;
    sf::base::SizeT catIdx;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Vec2f getDrawPosition() const
    {
        return position;
    }
};
