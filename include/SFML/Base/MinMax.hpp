#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& min(const T& a, const T& b) noexcept
{
    return b < a ? b : a;
}

////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& max(const T& a, const T& b) noexcept
{
    return a < b ? b : a;
}

} // namespace sf::base
