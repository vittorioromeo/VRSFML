#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename Comparer>
[[nodiscard, gnu::always_inline, gnu::pure]] ForwardIt maxElement(ForwardIt first, const ForwardIt last, Comparer&& comp)
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
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] ForwardIt maxElement(const ForwardIt first, const ForwardIt last)
{
    return maxElement(first, last, [](const auto& a, const auto& b) { return a < b; });
}

} // namespace sf::base
