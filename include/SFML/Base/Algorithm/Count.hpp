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
/// \brief Count the number of elements in a range that evaluate to true
///
/// Iterates through the range and increments a counter for each element
/// that, when converted to bool, is true.
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
///
/// \return The number of elements evaluating to true
///
////////////////////////////////////////////////////////////
template <typename Iter>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT countTruthy(Iter rangeBegin, Iter rangeEnd)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (static_cast<bool>(*rangeBegin))
            ++result;

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Count the number of elements in a range equal to a specific value
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param value      Value to compare elements against
///
/// \return The number of elements equal to value
///
////////////////////////////////////////////////////////////
template <typename Iter, typename T>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT count(Iter rangeBegin, Iter rangeEnd, const T& value)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (*rangeBegin == value)
            ++result;

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Count the number of elements in a range satisfying a predicate
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param predicate  Unary predicate function
///
/// \return The number of elements for which predicate returns true
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT countIf(Iter rangeBegin, Iter rangeEnd, Predicate&& predicate)
{
    SizeT result = 0u;

    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            ++result;

    return result;
}

} // namespace sf::base
