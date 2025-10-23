#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ShapeBlockSequence.hpp"
#include "TetraminoShapes.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/IntTypes.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
inline constexpr sf::base::Array<sf::Color, tetraminoShapeCount> blockPalette{
    sf::Color{13u, 194u, 255u}, // I - Cyan
    sf::Color{255u, 225u, 56u}, // O - Yellow
    sf::Color{245u, 56u, 255u}, // T - Purple
    sf::Color{255u, 142u, 13u}, // L - Orange
    sf::Color{56u, 119u, 255u}, // J - Blue
    sf::Color{13u, 255u, 114u}, // S - Green
    sf::Color{255u, 13u, 114u}  // Z - Red
};


////////////////////////////////////////////////////////////
enum class BlockPowerup : sf::base::U8
{
    None          = 0u,
    XPBonus       = 1u,
    ColumnDrill   = 2u,
    ThreeRowDrill = 3u,
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] Block
{
    sf::base::U32 tetraminoId;
    sf::base::U32 blockId;

    sf::base::U8       health;
    sf::base::U8       paletteIdx;
    ShapeBlockSequence shapeBlockSequence;

    BlockPowerup powerup;
};

} // namespace tsurv
