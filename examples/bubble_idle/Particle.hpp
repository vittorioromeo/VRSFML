#pragma once

#include "ParticleData.hpp"
#include "ParticleType.hpp"

#include "ZancleBase/IntTypes.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Particle : ParticleData
{
    zb::U8 hueByte = 0u;
    ParticleType type;
};
