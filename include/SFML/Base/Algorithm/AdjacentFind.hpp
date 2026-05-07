#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Find the first iterator `it` in `[first, last)` such that `predicate(*it, *(it + 1))`,
///        or `last` if no such pair exists
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
/// \brief `adjacentFind` overload using `operator==` to compare adjacent elements
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
[[nodiscard, gnu::always_inline]] inline constexpr ForwardIt adjacentFind(const ForwardIt first, const ForwardIt last)
{
    return adjacentFind(first, last, [](const auto& a, const auto& b) { return a == b; });
}

} // namespace sf::base
