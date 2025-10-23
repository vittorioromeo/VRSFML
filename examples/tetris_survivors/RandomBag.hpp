#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
template <typename Container>
void shuffleBag(Container& bag, auto&& rng) // TODO P1: to base algos
{
    const sf::base::SizeT n = bag.size();

    if (n <= 1)
        return;

    for (sf::base::SizeT i = n - 1; i > 0; --i)
    {
        const sf::base::SizeT j = rng.getI(sf::base::SizeT{0}, i);
        sf::base::swap(bag[i], bag[j]);
    }
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
