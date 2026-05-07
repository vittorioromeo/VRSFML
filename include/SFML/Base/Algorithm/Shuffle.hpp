#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Shuffle a range in-place using Fisher-Yates (Durstenfeld)
///
/// `rngFunc(min, max)` must return a uniform `SizeT` in `[min, max]`.
///
////////////////////////////////////////////////////////////
template <typename RandomIt, typename RngFunc>
[[gnu::always_inline]] constexpr void shuffle(const RandomIt rangeBegin, const RandomIt rangeEnd, RngFunc&& rngFunc)
{
    auto n = rangeEnd - rangeBegin;

    if (n <= 1)
        return;

    for (decltype(n) i = n - 1; i > 0; --i)
    {
        const auto j = rngFunc(SizeT{0u}, static_cast<SizeT>(i));
        SFML_BASE_ASSERT(j <= static_cast<SizeT>(i));

        iterSwap(rangeBegin + i, rangeBegin + j);
    }
}

} // namespace sf::base
