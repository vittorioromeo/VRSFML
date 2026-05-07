#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Count elements in the range that convert to `true`
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT countTruthy(ForwardIt rangeBegin, const ForwardIt rangeEnd)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (static_cast<bool>(*rangeBegin))
            ++result;

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Count elements in the range equal to `value`
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT count(ForwardIt rangeBegin, const ForwardIt rangeEnd, const T& value)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (*rangeBegin == value)
            ++result;

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Count elements in the range satisfying `predicate`
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT countIf(ForwardIt       rangeBegin,
                                                                     const ForwardIt rangeEnd,
                                                                     Predicate&&     predicate)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            ++result;

    return result;
}

} // namespace sf::base
