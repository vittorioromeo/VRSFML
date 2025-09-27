#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& clamp(const T& value, const T& minValue, const T& maxValue) noexcept
{
    // NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter)
    return (value < minValue) ? minValue : ((value > maxValue) ? maxValue : value);
}

} // namespace sf::base
