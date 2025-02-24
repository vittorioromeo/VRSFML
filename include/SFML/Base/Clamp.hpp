#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& clamp(const T& value, const T& minValue, const T& maxValue) noexcept
{
    return (value < minValue) ? minValue : ((value > maxValue) ? maxValue : value);
}

} // namespace sf::base
