#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "BlockMatrix.hpp"
#include "TetraminoShapes.hpp"

#include "SFML/System/Vec2.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct Tetramino
{
    BlockMatrix   shape;
    sf::Vec2i     position;
    TetraminoType tetraminoType;
    unsigned char rotationState; // [0-3]
};

} // namespace tsurv
