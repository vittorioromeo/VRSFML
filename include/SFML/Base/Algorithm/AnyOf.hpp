#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `true` if at least one element in the range satisfies `predicate`
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename Predicate>
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool anyOf(ForwardIt       rangeBegin,
                                                                  const ForwardIt rangeEnd,
                                                                  Predicate&&     predicate) noexcept
{
    for (; rangeBegin != rangeEnd; ++rangeBegin)
        if (predicate(*rangeBegin))
            return true;

    return false;
}

} // namespace sf::base
