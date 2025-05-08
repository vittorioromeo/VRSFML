#pragma once

#include "BubbleType.hpp"
#include "Countdown.hpp"

#include "SFML/System/Vec2.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Bubble
{
    sf::Vec2f position;
    sf::Vec2f velocity;

    float radius;
    float rotation;
    float hueMod;

    Countdown repelledCountdown;
    Countdown attractedCountdown;

    BubbleType type;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline float getRadiusSquared() const
    {
        return radius * radius;
    }
};
