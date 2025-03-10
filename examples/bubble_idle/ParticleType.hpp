#pragma once

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
enum class [[nodiscard]] ParticleType : sf::base::U8
{
    Bubble    = 0u,
    Star      = 1u,
    Fire      = 2u,
    Hex       = 3u,
    Shrine    = 4u,
    Cursor    = 5u,
    Cog       = 6u,
    Coin      = 7u,
    CatSoul   = 8u,
    Fire2     = 9u,
    Smoke     = 10u,
    Explosion = 11u,
    Trail     = 12u,

    Count
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto asIdx(const ParticleType type) noexcept
{
    return static_cast<sf::base::SizeT>(type);
}

////////////////////////////////////////////////////////////
enum : sf::base::SizeT
{
    nParticleTypes = asIdx(ParticleType::Count)
};
