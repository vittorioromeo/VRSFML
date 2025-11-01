#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Copy elements from one range to another
///
/// \param rangeBegin Iterator to the beginning of the source range
/// \param rangeEnd   Iterator to the end of the source range
/// \param targetIter Iterator to the beginning of the destination range
///
/// \return Iterator to the element past the last element copied
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
