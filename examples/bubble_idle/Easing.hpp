#pragma once

#include "AssertAssume.hpp"

#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/Math/Sin.hpp"

#include <iostream>


////////////////////////////////////////////////////////////
using EasingFn = float (*)(const float);

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeIdentity(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);
    return x;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeInOutCubic(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);
    return x < 0.5f ? 4.f * x * x * x : 1.f - sf::base::pow(-2.f * x + 2.f, 3.f) / 2.f;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeInOutSine(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);
    return -(sf::base::cos(sf::base::pi * x) - 1.f) / 2.f;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeInOutQuint(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);
    return x < 0.5f ? 16.f * x * x * x * x * x : 1.f - sf::base::pow(-2.f * x + 2.f, 5.f) / 2.f;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeInOutBack(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);

    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;

    return x < 0.5f ? (sf::base::pow(2.f * x, 2.f) * ((c2 + 1.f) * 2.f * x - c2)) / 2.f
                    : (sf::base::pow(2.f * x - 2.f, 2.f) * ((c2 + 1.f) * (x * 2.f - 2.f) + c2) + 2.f) / 2.f;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeInBack(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);

    const float c1 = 1.70158f;
    const float c3 = c1 + 1.f;

    return c3 * x * x * x - c1 * x * x;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeOutBack(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);

    const float c1 = 1.70158f;
    const float c3 = c1 + 1.f;

    return 1.f + c3 * sf::base::pow(x - 1.f, 3.f) + c1 * sf::base::pow(x - 1.f, 2.f);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeInOutElastic(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);

    const float c5 = (2.f * sf::base::pi) / 4.5f;

    return x == 0.f   ? 0.f
           : x == 1.f ? 1.f
           : x < 0.5f ? -(sf::base::pow(2.f, 20.f * x - 10.f) * sf::base::sin((20.f * x - 11.125f) * c5)) / 2.f
                      : (sf::base::pow(2.f, -20.f * x + 10.f) * sf::base::sin((20.f * x - 11.125f) * c5)) / 2.f + 1.f;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeInQuint(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);
    return x * x * x * x * x;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float easeOutQuint(const float x) noexcept
{
    ASSERT_AND_ASSUME(x >= 0.f && x <= 1.f);
    return 1.f - sf::base::pow(1.f - x, 5.f);
}
