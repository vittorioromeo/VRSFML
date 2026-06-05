#pragma once

#include "ParticleData.hpp"
#include "ParticleType.hpp"

#include "Zancle/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Particle : ParticleData
{
    za::U8       hueByte = 0u;
    ParticleType type;
};
