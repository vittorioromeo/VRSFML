#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "ZancleBase/AssertAndAssume.hpp"
#include "ZancleBase/Math/Exp.hpp"


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float remap(
    const float x,
    const float oldMin,
    const float oldMax,
    const float newMin,
    const float newMax) noexcept
{
    ZB_ASSERT_AND_ASSUME(oldMax != oldMin);
    return newMin + ((x - oldMin) / (oldMax - oldMin)) * (newMax - newMin);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float blend(
    const float a,
    const float b,
    const float value) noexcept
{
    return a + (b - a) * value;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr za::Vec2f blend(
    const za::Vec2f a,
    const za::Vec2f b,
    const float     value) noexcept
{
    return a + (b - a) * value;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float exponentialApproach(
    const float current,
    const float target,
    const float deltaTimeMs,
    const float timeToConverge) noexcept
{
    ZB_ASSERT_AND_ASSUME(timeToConverge > 0.f);

    const float factor = 1.f - zb::exp(-deltaTimeMs / timeToConverge);
    return current + (target - current) * factor;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr za::Vec2f exponentialApproach(
    const za::Vec2f current,
    const za::Vec2f target,
    const float     deltaTimeMs,
    const float     timeToConverge) noexcept
{
    ZB_ASSERT_AND_ASSUME(timeToConverge > 0.f);

    const float factor = 1.f - zb::exp(-deltaTimeMs / timeToConverge);
    return current + (target - current) * factor;
}
