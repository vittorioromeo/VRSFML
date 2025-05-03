#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
