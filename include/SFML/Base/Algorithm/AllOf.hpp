#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `true` if all elements in the range satisfy `predicate`
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
