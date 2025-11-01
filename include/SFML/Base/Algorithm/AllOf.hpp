#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Check if all elements in a range satisfy a predicate
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param predicate  Unary predicate function
///
/// \return `true` if predicate returns true for all elements, `false` otherwise
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool allOf(ForwardIt       rangeBegin,
                                                                  const ForwardIt rangeEnd,
                                                                  Predicate&&     predicate) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (!predicate(*rangeBegin))
            return false;

    return true;
}

} // namespace sf::base
