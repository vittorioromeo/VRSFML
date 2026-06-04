#pragma once

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
enum class [[nodiscard]] BubbleType : zb::U8
{
    Normal = 0u,
    Star   = 1u,
    Bomb   = 2u,
    Nova   = 3u,
    Combo  = 4u, // event-spawned; invincible (cannot be popped/destroyed)

    Count
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto asIdx(const BubbleType type) noexcept
{
    return static_cast<zb::SizeT>(type);
}

////////////////////////////////////////////////////////////
enum : zb::SizeT
{
    nBubbleTypes = asIdx(BubbleType::Count)
};
