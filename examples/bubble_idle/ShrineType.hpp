#pragma once

#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
enum class ShrineType : za::U8
{
    Voodoo     = 0u,
    Magic      = 1u,
    Clicking   = 2u,
    Automation = 3u,
    Repulsion  = 4u,
    Attraction = 5u,
    Camouflage = 6u,
    Victory    = 7u,

    Count
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto asIdx(const ShrineType catType) noexcept
{
    return static_cast<za::SizeT>(catType);
}

////////////////////////////////////////////////////////////
enum : za::SizeT
{
    nShrineTypes = asIdx(ShrineType::Count)
};
