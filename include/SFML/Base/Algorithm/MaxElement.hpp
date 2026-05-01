#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Find the largest element in a range using a custom comparator
///
/// Returns an iterator to the greatest element in the range `[first, last)`
/// according to the strict weak ordering imposed by `comp`. If several
/// elements compare equal to the greatest, the iterator to the first such
/// element is returned. If the range is empty, `last` is returned.
///
/// \param first Iterator to the beginning of the range
/// \param last  Iterator to the end of the range
/// \param comp  Binary comparator returning `true` if its first argument is less than its second
///
/// \return Iterator to the greatest element, or `last` if the range is empty
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
/// \brief Find the largest element in a range using `operator<`
///
/// Equivalent to calling the comparator overload with `[](a, b) { return a < b; }`.
///
/// \param first Iterator to the beginning of the range
/// \param last  Iterator to the end of the range
///
/// \return Iterator to the greatest element, or `last` if the range is empty
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr ForwardIt maxElement(const ForwardIt first,
                                                                                          const ForwardIt last)
{
    return maxElement(first, last, [](const auto& a, const auto& b) { return a < b; });
}

} // namespace sf::base
