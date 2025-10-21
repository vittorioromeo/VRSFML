#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "BlockMatrix.hpp"
#include "TetraminoShapes.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct Tetramino // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    BlockMatrix   shape;
    sf::Vec2i     position;
    TetraminoType tetraminoType;
    sf::base::U8  rotationState; // [0-3]
};

} // namespace tsurv
