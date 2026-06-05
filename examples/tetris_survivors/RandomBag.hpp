#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Algorithm/Shuffle.hpp"
#include "Zancle/Base/SizeT.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
template <typename RandomIt>
constexpr void shuffleBag(const RandomIt begin, const RandomIt end, auto&& rng)
{
    za::shuffle(begin, end, [&rng](za::SizeT min, za::SizeT max) { return rng.getI(min, max); });
}


////////////////////////////////////////////////////////////
template <typename Container>
constexpr void shuffleBag(Container& bag, auto&& rng)
{
    shuffleBag(bag.begin(), bag.end(), rng);
}

} // namespace tsurv
