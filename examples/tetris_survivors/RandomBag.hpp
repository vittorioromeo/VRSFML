#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Algorithm/Shuffle.hpp"
#include "ZancleBase/SizeT.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
template <typename RandomIt>
constexpr void shuffleBag(const RandomIt begin, const RandomIt end, auto&& rng)
{
    zb::shuffle(begin, end, [&rng](zb::SizeT min, zb::SizeT max) { return rng.getI(min, max); });
}


////////////////////////////////////////////////////////////
template <typename Container>
constexpr void shuffleBag(Container& bag, auto&& rng)
{
    shuffleBag(bag.begin(), bag.end(), rng);
}

} // namespace tsurv
