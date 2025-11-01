#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Algorithm/Shuffle.hpp"
#include "SFML/Base/SizeT.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
template <typename RandomIt>
constexpr void shuffleBag(const RandomIt begin, const RandomIt end, auto&& rng)
{
    sf::base::shuffle(begin, end, [&rng](sf::base::SizeT min, sf::base::SizeT max) { return rng.getI(min, max); });
}


////////////////////////////////////////////////////////////
template <typename Container>
constexpr void shuffleBag(Container& bag, auto&& rng)
{
    shuffleBag(bag.begin(), bag.end(), rng);
}

} // namespace tsurv
