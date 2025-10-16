#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Check if a range is sorted according to a comparison function
///
/// \param first Iterator to the beginning of the range
/// \param last  Iterator to the end of the range
/// \param comp  Binary comparison function object
///
/// \return `true` if the range is sorted, `false` otherwise
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Comparer>
[[nodiscard]] constexpr bool isSorted(Iter first, Iter last, Comparer&& comp)
{
    if (first != last)
    {
        Iter next = first;
        while (++next != last)
        {
            if (comp(*next, *first))
                return false;

            first = next;
        }
    }

    return true;
}

} // namespace sf::base
