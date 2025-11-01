#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Finds the first pair of adjacent elements in a range that satisfy a predicate.
///
/// Searches the range `[first, last)` and returns an iterator to the first element `it`
/// such that `predicate(*it, *(it + 1))` is `true`.
///
/// \tparam ForwardIt       The type of the iterators, must meet the requirements of a forward iterator.
/// \tparam BinaryPredicate The type of the predicate function object.
///
/// \param first     Iterator to the beginning of the range to search.
/// \param last      Iterator to the end of the range to search.
/// \param predicate A binary predicate that takes two arguments and returns a value convertible to `bool`.
///                  The predicate is applied to adjacent elements `(a, b)`.
///
/// \return An iterator to the first element of the first pair satisfying the predicate.
///         If no such pair is found, `last` is returned.
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename BinaryPredicate>
[[nodiscard]] constexpr ForwardIt adjacentFind(ForwardIt first, const ForwardIt last, BinaryPredicate&& predicate)
{
    if (first == last)
        return last; // Empty range

    ForwardIt next = first;
    ++next;

    while (next != last)
    {
        if (predicate(*first, *next))
            return first; // Found a pair

        ++first;
        ++next;
    }

    return last; // No pair found
}


////////////////////////////////////////////////////////////
/// \brief Finds the first pair of equal adjacent elements in a range.
///
/// This is an overload of `adjacentFind` that uses the equality operator (`==`)
/// to compare adjacent elements. It returns an iterator to the first element `it`
/// such that `*it == *(it + 1)` is `true`.
///
/// \tparam ForwardIt The type of the iterators, must meet the requirements of a forward iterator.
///
/// \param first Iterator to the beginning of the range to search.
/// \param last  Iterator to the end of the range to search.
///
/// \return An iterator to the first element of the first pair of equal adjacent elements.
///         If no such pair is found, `last` is returned.
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
[[nodiscard, gnu::always_inline]] inline constexpr ForwardIt adjacentFind(const ForwardIt first, const ForwardIt last)
{
    return sf::base::adjacentFind(first, last, [](const auto& a, const auto& b) { return a == b; });
}

} // namespace sf::base
