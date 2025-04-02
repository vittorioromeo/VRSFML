#pragma once

#include "BubbleType.hpp"
#include "Countdown.hpp"

#include "SFML/System/Vector2.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Bubble
{
    sf::Vector2f position;
    sf::Vector2f velocity;

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
