#pragma once

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Math/Exp.hpp"


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float remap(
    const float x,
    const float oldMin,
    const float oldMax,
    const float newMin,
    const float newMax) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(oldMax != oldMin);
    return newMin + ((x - oldMin) / (oldMax - oldMin)) * (newMax - newMin);
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float blend(
    const float a,
    const float b,
    const float value) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(value >= 0.f && value <= 1.f);
    return a + (b - a) * value;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float exponentialApproach(
    const float current,
    const float target,
    const float deltaTimeMs,
    const float speed) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(speed >= 0.f);

    const float factor = 1.f - sf::base::exp(-deltaTimeMs / speed);
    return current + (target - current) * factor;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr sf::Vec2f exponentialApproach(
    const sf::Vec2f current,
    const sf::Vec2f target,
    const float     deltaTimeMs,
    const float     speed) noexcept
{
    // TODO P2: this is axis independent, should be fixed

    return {exponentialApproach(current.x, target.x, deltaTimeMs, speed),
            exponentialApproach(current.y, target.y, deltaTimeMs, speed)};
}
