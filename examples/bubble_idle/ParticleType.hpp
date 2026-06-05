#pragma once

#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
enum class [[nodiscard]] ParticleType : za::U8
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
    Glass     = 13u,

    Count
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto asIdx(const ParticleType type) noexcept
{
    return static_cast<za::SizeT>(type);
}

////////////////////////////////////////////////////////////
enum : za::SizeT
{
    nParticleTypes = asIdx(ParticleType::Count)
};
