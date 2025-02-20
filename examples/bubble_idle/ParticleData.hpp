#pragma once

#include "SFML/System/Vector2.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] ParticleData
{
    sf::Vector2f position;
    sf::Vector2f velocity;

    float scale; // TODO P2: consider optimizing memory layout
    float accelerationY;

    float opacity;
    float opacityDecay;

    float rotation;
    float torque;
};
