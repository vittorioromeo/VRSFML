#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Copy `[rangeBegin, rangeEnd)` to `targetIter`, returning the destination end iterator
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename TargetForwardIt>
[[gnu::always_inline]] constexpr TargetForwardIt copy(ForwardIt rangeBegin, const ForwardIt rangeEnd, TargetForwardIt targetIter)
{
    while (rangeBegin != rangeEnd)
        *targetIter++ = *rangeBegin++;

    return targetIter;
}

} // namespace sf::base
