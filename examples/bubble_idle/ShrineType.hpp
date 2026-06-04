#pragma once

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
enum class ShrineType : zb::U8
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
    return static_cast<zb::SizeT>(catType);
}

////////////////////////////////////////////////////////////
enum : zb::SizeT
{
    nShrineTypes = asIdx(ShrineType::Count)
};
