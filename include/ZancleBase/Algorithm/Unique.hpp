#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Macros.hpp"


namespace zb
{
////////////////////////////////////////////////////////////
/// \brief Remove all but the first element from each consecutive group of equal elements
///
/// Survivors are shifted to the front of the range; elements past the
/// returned iterator are in a valid but unspecified state.
///
/// \return Iterator to the new logical end of the range
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
constexpr ForwardIt unique(ForwardIt first, const ForwardIt last)
{
    if (first == last)
        return last;

    ForwardIt result = first;

    while (++first != last)
        if (!(*result == *first) && ++result != first) // NOLINT(bugprone-inc-dec-in-conditions)
            *result = ZB_MOVE(*first);

    return ++result;
}

} // namespace zb
