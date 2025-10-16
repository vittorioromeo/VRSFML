#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Check if any element in a range satisfies a predicate
///
/// \param rangeBegin Iterator to the beginning of the range
/// \param rangeEnd   Iterator to the end of the range
/// \param predicate  Unary predicate function
///
/// \return `true` if predicate returns true for at least one element, `false` otherwise
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool anyOf(Iter rangeBegin, Iter rangeEnd, Predicate&& predicate) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            return true;

    return false;
}

} // namespace sf::base
