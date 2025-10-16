#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename BinaryPredicate>
ForwardIt adjacentFind(ForwardIt first, const ForwardIt last, BinaryPredicate&& p)
{
    if (first == last)
        return last; // Empty range

    ForwardIt next = first;
    ++next;

    while (next != last)
    {
        if (p(*first, *next))
            return first; // Found a pair

        ++first;
        ++next;
    }

    return last; // No pair found
}


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
[[gnu::always_inline]] inline ForwardIt adjacentFind(const ForwardIt first, const ForwardIt last)
{
    return sf::base::adjacentFind(first, last, [](const auto& a, const auto& b) { return a == b; });
}

} // namespace sf::base
