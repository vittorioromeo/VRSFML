#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"
#include "DrillDirection.hpp"
#include "LaserDirection.hpp"
#include "Tetramino.hpp"
#include "ZancleBase/InPlaceVector.hpp"
#include "ZancleBase/Variant.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimWait
{
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimHardDrop // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino tetramino;
    int       endY;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimSquish // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino tetramino;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimClearLines
{
    using RowVector = zb::InPlaceVector<zb::SizeT, 8>;

    RowVector rows;
    bool      awardXP;
    bool      forceClear;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimFadeBlocks
{
    struct FadingBlock // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        Block      block;
        za::Vec2uz position;
    };

    using FadingBlockVector = zb::InPlaceVector<FadingBlock, 64>;

    FadingBlockVector fadingBlocks;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimCollapseGrid
{
    using RowVector = AnimClearLines::RowVector;

    RowVector clearedRows;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimDrill // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino            tetramino;
    DrillDirection::Enum direction;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimColumnClear // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    za::Vec2uz position;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimLaser // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino            tetramino;
    LaserDirection::Enum direction;
    za::Vec2i            gridStartPos;
    za::Vec2i            gridTargetPos;
    bool                 onlyVisual;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimFadeAttachments // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino tetramino;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimLightningStrike // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    zb::SizeT numStrikes;
};


////////////////////////////////////////////////////////////
struct AnimationCommandP0
{
    using VariantType = zb::Variant< //
        AnimWait,
        AnimDrill,
        AnimLightningStrike,
        AnimLaser,
        AnimFadeAttachments>;

    VariantType data;
    float       duration;
};


////////////////////////////////////////////////////////////
struct AnimationCommandP1
{
    using VariantType = zb::Variant< //
        AnimHardDrop,
        AnimSquish,
        AnimClearLines,
        AnimFadeBlocks,
        AnimCollapseGrid,
        AnimColumnClear>;

    VariantType data;
    float       duration;
};


////////////////////////////////////////////////////////////
struct AnimationCommandP2
{
    using VariantType = zb::Variant< //
        AnimClearLines>;

    VariantType data;
    float       duration;
};

} // namespace tsurv
