#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline constexpr void swap(T& a, T& b) noexcept
{
    T tempA = static_cast<T&&>(a);
    a       = static_cast<T&&>(b);
    b       = static_cast<T&&>(tempA);
}


////////////////////////////////////////////////////////////
template <typename ForwardIt1, typename ForwardIt2>
[[gnu::always_inline]] inline constexpr void iterSwap(const ForwardIt1 a, const ForwardIt2 b)
{
    using base::swap;
    swap(*a, *b);
}


////////////////////////////////////////////////////////////
template <typename ForwardIt1, typename ForwardIt2>
[[gnu::always_inline]] inline constexpr ForwardIt2 swapRanges(ForwardIt1 first1, const ForwardIt1 last1, ForwardIt2 first2)
{
    for (; first1 != last1; ++first1, ++first2)
        iterSwap(first1, first2);

    return first2;
}

} // namespace sf::base
