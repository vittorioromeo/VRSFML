#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Iterator to the greatest element under `comp` (first such if ties), or `last` if empty
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename Comparer>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr ForwardIt maxElement(ForwardIt first, const ForwardIt last, Comparer&& comp)
{
    if (first == last)
        return last;

    ForwardIt largest = first;

    while (++first != last)
        if (comp(*largest, *first))
            largest = first;

    return largest;
}


////////////////////////////////////////////////////////////
/// \brief `maxElement` overload using `operator<`
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr ForwardIt maxElement(const ForwardIt first,
                                                                                          const ForwardIt last)
{
    return maxElement(first, last, [](const auto& a, const auto& b) { return a < b; });
}

} // namespace sf::base
