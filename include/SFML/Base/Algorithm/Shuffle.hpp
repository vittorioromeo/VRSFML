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
/// \brief Randomly shuffles the elements in a range using the Fisher-Yates algorithm.
///
/// Rearranges the elements in the range `[rangeBegin, rangeEnd)` into a pseudo-random
/// order. This implementation uses the Durstenfeld variation of the Fisher-Yates shuffle,
/// which guarantees a uniform permutation of the elements, provided that the supplied
/// random number generator is uniform.
///
/// \tparam RandomIt The type of the iterators, must meet the requirements of a random access iterator.
/// \tparam RngFunc        The type of the random number generator function object.
///
/// \param rangeBegin Iterator to the beginning of the range to shuffle.
/// \param rangeEnd   Iterator to the end of the range to shuffle.
/// \param rngFunc    A function object that takes two arguments `(min, max)` of type `sf::base::SizeT`
///                   and returns a random integer in the inclusive range `[min, max]`.
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
        iterSwap(rangeBegin + i, rangeBegin + j);
    }
}

} // namespace sf::base
