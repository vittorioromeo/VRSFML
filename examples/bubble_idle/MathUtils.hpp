#pragma once

#include "AssertAssume.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Math/Exp.hpp"
#include "SFML/Base/Remainder.hpp"


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float remap(
    const float x,
    const float oldMin,
    const float oldMax,
    const float newMin,
    const float newMax) noexcept
{
    ASSERT_AND_ASSUME(oldMax != oldMin);
    return newMin + ((x - oldMin) / (oldMax - oldMin)) * (newMax - newMin);
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float blend(
    const float a,
    const float b,
    const float value) noexcept
{
    ASSERT_AND_ASSUME(value >= 0.f && value <= 1.f);
    return a + (b - a) * value;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float exponentialApproach(
    const float current,
    const float target,
    const float deltaTimeMs,
    const float speed) noexcept
{
    ASSERT_AND_ASSUME(speed >= 0.f);

    const float factor = 1.f - sf::base::exp(-deltaTimeMs / speed);
    return current + (target - current) * factor;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr sf::Vector2f exponentialApproach(
    const sf::Vector2f current,
    const sf::Vector2f target,
    const float        deltaTimeMs,
    const float        speed) noexcept
{
    // TODO P2: this is axis independent, should be fixed

    return {exponentialApproach(current.x, target.x, deltaTimeMs, speed),
            exponentialApproach(current.y, target.y, deltaTimeMs, speed)};
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float wrapHue(const float hue) noexcept
{
    return sf::base::positiveRemainder(hue, 360.f);
}
