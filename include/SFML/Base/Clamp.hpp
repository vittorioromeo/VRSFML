#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Clamp `value` into the closed range `[minValue, maxValue]`
///
/// \return Reference to `value` if in range, otherwise to the closer bound
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& clamp(const T& value, const T& minValue, const T& maxValue) noexcept
{
    SFML_BASE_ASSERT(minValue <= maxValue);

    // NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter)
    return (value < minValue) ? minValue : ((value > maxValue) ? maxValue : value);
}

} // namespace sf::base
