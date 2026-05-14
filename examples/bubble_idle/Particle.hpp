#pragma once

#include "ParticleData.hpp"
#include "ParticleType.hpp"

#include "SFML/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Particle : ParticleData
{
    sf::base::U8 hueByte = 0u;
    ParticleType type;
};
