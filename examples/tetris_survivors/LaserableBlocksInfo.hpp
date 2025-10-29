#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/InPlaceVector.hpp"


namespace tsurv
{
/////////////////////////////////////////////////////////////
struct [[nodiscard]] LaserableBlocksInfo
{
    sf::base::InPlaceVector<sf::Vec2i, 32> positions;
    sf::base::InPlaceVector<sf::Vec2i, 32> bouncePositions;
    sf::Vec2i                              bouncePos;
};

} // namespace tsurv
