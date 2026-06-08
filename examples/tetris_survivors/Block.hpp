#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ShapeBlockSequence.hpp"
#include "StrongTypedef.hpp"
#include "TetraminoShapes.hpp"

#include "Zancle/Graphics/Color.hpp"

#include "Zancle/Container/Array.hpp"

#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/IntTypes.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
inline constexpr za::Array<za::Color, tetraminoShapeCount> blockPalette{
    za::Color{13u, 194u, 255u}, // I - Cyan
    za::Color{255u, 225u, 56u}, // O - Yellow
    za::Color{245u, 56u, 255u}, // T - Purple
    za::Color{255u, 142u, 13u}, // L - Orange
    za::Color{56u, 119u, 255u}, // J - Blue
    za::Color{13u, 255u, 114u}, // S - Green
    za::Color{255u, 13u, 114u}  // Z - Red
};


////////////////////////////////////////////////////////////
enum class [[nodiscard]] BlockPowerup : za::U8
{
    None          = 0u,
    XPBonus       = 1u,
    ColumnDrill   = 2u,
    ThreeRowDrill = 3u,
};


////////////////////////////////////////////////////////////
TSURV_DEFINE_STRONG_TYPEDEF(BlockId, za::U16);
TSURV_DEFINE_STRONG_TYPEDEF(TetraminoId, za::U16);
TSURV_DEFINE_STRONG_TYPEDEF(Health, za::U8);
TSURV_DEFINE_STRONG_TYPEDEF(PaletteIdx, za::U8);


////////////////////////////////////////////////////////////
inline constexpr auto nullTickTimerTarget = static_cast<za::U32>(-1);


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
    za::U32 tickTimer;
    za::U32 tickTimerTarget;

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
        ZA_ASSERT(isDamageable());

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
