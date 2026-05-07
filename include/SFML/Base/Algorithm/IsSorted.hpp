#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `true` if the range is sorted according to `comp`
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename Comparer>
[[nodiscard]] constexpr bool isSorted(ForwardIt first, const ForwardIt last, Comparer&& comp)
{
    if (first == last)
        return true;

    ForwardIt next = first;
    while (++next != last)
    {
        if (comp(*next, *first))
            return false;

        first = next;
    }

    return true;
}

} // namespace sf::base
