#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] constexpr void swap(T& a, T& b) noexcept
{
    T tempA = static_cast<T&&>(a);
    a       = static_cast<T&&>(b);
    b       = static_cast<T&&>(tempA);
}


////////////////////////////////////////////////////////////
template <typename Iter1, typename Iter2>
[[gnu::always_inline]] constexpr void iterSwap(Iter1 a, Iter2 b)
{
    using base::swap;
    swap(*a, *b);
}


////////////////////////////////////////////////////////////
template <typename Iter1, typename Iter2>
[[gnu::always_inline]] constexpr Iter2 swapRanges(Iter1 first1, Iter1 last1, Iter2 first2)
{
    for (; first1 != last1; ++first1, ++first2)
        iterSwap(first1, first2);

    return first2;
}

} // namespace sf::base
