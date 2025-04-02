#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/AssertAndAssume.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float remainder(const float a, const float b) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(b > 0.f);

    return a - static_cast<float>(static_cast<int>(a / b)) * b;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float positiveRemainder(const float a, const float b) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(b > 0.f);

    const auto val = a - static_cast<float>(static_cast<int>(a / b)) * b;
    return val >= 0.f ? val : val + b;
}

} // namespace sf::base
