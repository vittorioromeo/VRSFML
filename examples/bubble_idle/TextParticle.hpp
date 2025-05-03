#pragma once

#include "ParticleData.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] TextParticle : ParticleData
{
    char buffer[16]{};
};
