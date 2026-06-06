#pragma once

#include "Zancle/Diagnostic/Assert.hpp"

#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
enum class [[nodiscard]] CatType : za::U8
{
    Normal = 0u,
    Uni    = 1u,
    Devil  = 2u,
    Astro  = 3u,
    Warden = 4u, // wakes up sleeping cats in their range

    Witch    = 5u,
    Wizard   = 6u,
    Mouse    = 7u,
    Engi     = 8u,
    Repulso  = 9u,
    Attracto = 10u,
    Copy     = 11u,
    Duck     = 12u,

    Count
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto asIdx(const CatType type) noexcept
{
    return static_cast<za::SizeT>(type);
}

////////////////////////////////////////////////////////////
enum : za::SizeT
{
    nCatTypes = asIdx(CatType::Count)
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr bool isUniqueCatType(const CatType catType) noexcept
{
    ZA_ASSERT(catType < CatType::Count);
    return catType >= CatType::Witch;
}
