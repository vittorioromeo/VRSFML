#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Return a reference to the smaller of two values
///
/// Equivalent to `std::min` but does not include `<algorithm>`.
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& min(const T& a, const T& b) noexcept
{
    // NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter)
    return b < a ? b : a;
}


////////////////////////////////////////////////////////////
/// \brief Return a reference to the larger of two values
///
/// Equivalent to `std::max` but does not include `<algorithm>`.
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& max(const T& a, const T& b) noexcept
{
    // NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter)
    return a < b ? b : a;
}

} // namespace sf::base
