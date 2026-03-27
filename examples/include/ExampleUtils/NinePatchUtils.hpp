#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Array.hpp"


////////////////////////////////////////////////////////////
/// \brief Border thickness configuration for a nine-patch rectangle.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] NinePatchBorders
{
    ////////////////////////////////////////////////////////////
    float left{};
    float right{};
    float top{};
    float bottom{};


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] static constexpr NinePatchBorders all(const float value) noexcept
    {
        return {value, value, value, value};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] static constexpr NinePatchBorders symmetric(
        const float horizontal,
        const float vertical) noexcept
    {
        return {horizontal, horizontal, vertical, vertical};
    }
};


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float clampNinePatchNonNegative(
    const float value) noexcept
{
    return value < 0.f ? 0.f : value;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr sf::base::Array<float, 3> makeNinePatchSlices(
    const float total,
    const float start,
    const float end) noexcept
{
    const float safeTotal = clampNinePatchNonNegative(total);
    float       first     = clampNinePatchNonNegative(start);
    float       third     = clampNinePatchNonNegative(end);

    const float borderSum = first + third;

    if (borderSum > safeTotal && borderSum > 0.f)
    {
        const float factor = safeTotal / borderSum;
        first *= factor;
        third *= factor;
    }

    return {first, safeTotal - first - third, third};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr sf::base::Array<float, 3> makeNinePatchPositions(
    const float origin,
    const sf::base::Array<float, 3>& slices) noexcept
{
    return {origin, origin + slices[0], origin + slices[0] + slices[1]};
}
