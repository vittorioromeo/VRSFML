#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename Iter, typename RngFunc>
[[gnu::always_inline]] constexpr void shuffle(Iter rangeBegin, Iter rangeEnd, RngFunc&& rngFunc)
{
    auto n = rangeEnd - rangeBegin;

    if (n <= 1)
        return;

    for (decltype(n) i = n - 1; i > 0; --i)
    {
        const auto j = rngFunc(SizeT{0u}, static_cast<SizeT>(i));
        iterSwap(rangeBegin + i, rangeBegin + j);
    }
}

} // namespace sf::base
