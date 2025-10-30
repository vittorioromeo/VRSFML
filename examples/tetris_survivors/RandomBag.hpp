#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Algorithm/Shuffle.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
template <typename Iter>
void shuffleBag(Iter begin, Iter end, auto&& rng)
{
    sf::base::shuffle(begin, end, [&rng](sf::base::SizeT min, sf::base::SizeT max) { return rng.getI(min, max); });
}


////////////////////////////////////////////////////////////
template <typename Container>
void shuffleBag(Container& bag, auto&& rng)
{
    shuffleBag(bag.begin(), bag.end(), rng);
}


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] T drawFromBag(sf::base::Vector<T>& bag)
{
    SFML_BASE_ASSERT(!bag.empty());

    auto item = bag.back();
    bag.popBack();

    return item;
}

} // namespace tsurv
