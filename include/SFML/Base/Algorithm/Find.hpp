#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Find the first element equal to a target value in a range
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param target     Value to search for
///
/// \return Iterator to the first element equal to target, or `rangeEnd` if not found
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr ForwardIt find(ForwardIt       rangeBegin,
                                                                      const ForwardIt rangeEnd,
                                                                      const T&        target) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (*rangeBegin == target)
            return rangeBegin;

    return rangeEnd;
}


////////////////////////////////////////////////////////////
/// \brief Find the first element satisfying a predicate in a range
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param predicate  Unary predicate function
///
/// \return Iterator to the first element for which predicate returns true, or `rangeEnd` if not found
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr ForwardIt findIf(ForwardIt       rangeBegin,
                                                                        const ForwardIt rangeEnd,
                                                                        Predicate&&     predicate) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            return rangeBegin;

    return rangeEnd;
}

} // namespace sf::base
