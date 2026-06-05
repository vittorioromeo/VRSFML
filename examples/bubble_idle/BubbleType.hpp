#pragma once

#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
enum class [[nodiscard]] BubbleType : za::U8
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
    return static_cast<za::SizeT>(type);
}

////////////////////////////////////////////////////////////
enum : za::SizeT
{
    nBubbleTypes = asIdx(BubbleType::Count)
};
