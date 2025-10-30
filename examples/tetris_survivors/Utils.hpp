#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Floor.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr sf::Vec2f floorVec2(const sf::Vec2<T> vec) noexcept
{
    return {sf::base::floor(vec.x), sf::base::floor(vec.y)};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float bounce(const float value) noexcept
{
    return 1.f - SFML_BASE_MATH_FABSF(value - 0.5f) * 2.f;
}

} // namespace tsurv
