#pragma once

#include "Zancle/Geometry/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] ParticleData
{
    za::Vec2f position;
    za::Vec2f velocity;

    float scale; // TODO P2: consider optimizing memory layout
    float scaleDecay;

    float accelerationY;

    float opacity;
    float opacityDecay;

    float rotation;
    float torque;
};
