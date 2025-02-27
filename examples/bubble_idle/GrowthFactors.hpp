#pragma once

#include "SFML/Base/Math/Pow.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] GrowthFactors
{
    const float initial;
    const float linear         = 0.f;
    const float multiplicative = 0.f;
    const float exponential    = 1.f;
    const float flat           = 0.f;
    const float finalMult      = 1.f;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float computeGrowth(const float n) const
    {
        return ((initial + n * multiplicative) * sf::base::pow(exponential, n) + linear * n + flat) * finalMult;
    }
};
