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
/// Equivalent to `std::clamp`, but does not require `<algorithm>`.
/// Asserts in debug builds that `minValue <= maxValue`.
///
/// \param value    Value to clamp
/// \param minValue Lower bound
/// \param maxValue Upper bound
///
/// \return Reference to `value` if in range, otherwise the closer bound
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
