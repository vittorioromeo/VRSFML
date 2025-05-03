#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] constexpr void swap(T& a, T& b) noexcept
{
    T tempA = SFML_BASE_MOVE(a);
    a       = SFML_BASE_MOVE(b);
    b       = SFML_BASE_MOVE(tempA);
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
