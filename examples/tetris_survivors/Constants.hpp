#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/SizeT.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
inline constexpr sf::Vec2f       resolution{320.f, 240.f};
inline constexpr float           ticksPerSecond = 120.f;
inline constexpr sf::base::SizeT gridGraceY     = 4u;


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
