#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Base/SizeT.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
inline constexpr za::Vec2f resolution{320.f, 240.f};
inline constexpr float     ticksPerSecond = 120.f;
inline constexpr za::SizeT gridGraceY     = 4u;


////////////////////////////////////////////////////////////
[[nodiscard]] inline float millisecondsToTicks(const float milliseconds)
{
    return milliseconds * (ticksPerSecond / 1000.f);
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline float secondsToTicks(const float seconds)
{
    return seconds * ticksPerSecond;
}

} // namespace tsurv
