#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ShapeBlockSequence.hpp"
#include "StrongTypedef.hpp"
#include "TetraminoShapes.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Assert.hpp"
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
enum class [[nodiscard]] BlockPowerup : sf::base::U8
{
    None          = 0u,
    XPBonus       = 1u,
    ColumnDrill   = 2u,
    ThreeRowDrill = 3u,
};


////////////////////////////////////////////////////////////
TSURV_DEFINE_STRONG_TYPEDEF(BlockId, sf::base::U16);
TSURV_DEFINE_STRONG_TYPEDEF(TetraminoId, sf::base::U16);
TSURV_DEFINE_STRONG_TYPEDEF(Health, sf::base::U8);
TSURV_DEFINE_STRONG_TYPEDEF(PaletteIdx, sf::base::U8);


////////////////////////////////////////////////////////////
inline constexpr auto nullTickTimerTarget = static_cast<sf::base::U32>(-1);


////////////////////////////////////////////////////////////
struct [[nodiscard]] Block
{
    ////////////////////////////////////////////////////////////
    TetraminoId tetraminoId;
    BlockId     blockId;

    ////////////////////////////////////////////////////////////
    Health     health;
    PaletteIdx paletteIdx;

    ////////////////////////////////////////////////////////////
    ShapeBlockSequence shapeBlockSequence;

    ////////////////////////////////////////////////////////////
    BlockPowerup powerup;

    ////////////////////////////////////////////////////////////
    sf::base::U32 tickTimer;
    sf::base::U32 tickTimerTarget;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isTransformingToArmored() const
    {
        return health == 1u && tickTimerTarget != nullTickTimerTarget && powerup == BlockPowerup::None;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isArmored() const
    {
        return health > 1u;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDamageable() const
    {
        return isArmored() || isTransformingToArmored();
    }

    ////////////////////////////////////////////////////////////
    void applyDamage()
    {
        SFML_BASE_ASSERT(isDamageable());

        if (isTransformingToArmored())
            tickTimerTarget = nullTickTimerTarget;
        else
            --health;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getTimerProgress() const
    {
        if (tickTimerTarget == nullTickTimerTarget)
            return 0.f;

        return static_cast<float>(tickTimer) / static_cast<float>(tickTimerTarget);
    }
};

} // namespace tsurv
