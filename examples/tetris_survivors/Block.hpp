#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ShapeMatrix.hpp"
#include "TetraminoShapes.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
inline constexpr sf::base::Array<sf::Color, tetraminoShapeCount> blockPalette = {
    sf::Color(13, 194, 255), // I - Cyan
    sf::Color(255, 225, 56), // O - Yellow
    sf::Color(245, 56, 255), // T - Purple
    sf::Color(255, 142, 13), // L - Orange
    sf::Color(56, 119, 255), // J - Blue
    sf::Color(13, 255, 114), // S - Green
    sf::Color(255, 13, 114)  // Z - Red
};


////////////////////////////////////////////////////////////
struct Block
{
    int             health;
    sf::base::SizeT paletteIdx;
    ShapeBlock      shapeBlock;
};

} // namespace tsurv
