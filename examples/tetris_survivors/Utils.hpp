#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Fabs.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float bounce(const float value) noexcept
{
    return 1.f - ZB_MATH_FABSF(value - 0.5f) * 2.f;
}

} // namespace tsurv
