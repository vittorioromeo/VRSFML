#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Fabs.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float bounce(const float value) noexcept
{
    return 1.f - SFML_BASE_MATH_FABSF(value - 0.5f) * 2.f;
}

} // namespace tsurv
