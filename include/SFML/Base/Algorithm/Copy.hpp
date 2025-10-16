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
template <typename Iter, typename TargetIter>
[[gnu::always_inline]] constexpr TargetIter copy(Iter rangeBegin, Iter rangeEnd, TargetIter targetIter)
{
    while (rangeBegin != rangeEnd)
        *targetIter++ = *rangeBegin++;

    return targetIter;
}

} // namespace sf::base
