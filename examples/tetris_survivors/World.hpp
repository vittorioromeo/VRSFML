#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"
#include "BlockGrid.hpp"
#include "BlockMatrix.hpp"
#include "Constants.hpp"
#include "DrillDirection.hpp"
#include "LaserDirection.hpp"
#include "RandomBag.hpp"
#include "Tetramino.hpp"
#include "TetraminoShapes.hpp"
#include "ZancleBase/Array.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Math/Pow.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] TaggedBlockMatrix // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    BlockMatrix   blockMatrix;
    TetraminoType tetraminoType;
};


////////////////////////////////////////////////////////////
[[nodiscard]] inline zb::U64 getXPNeededForLevelUp(const zb::U32 level)
{
    constexpr double baseXP   = 30.0;
    constexpr double exponent = 1.035;

    return static_cast<zb::U64>(baseXP * zb::pow(static_cast<double>(level), exponent));
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline zb::U64 getElapsedSeconds(const zb::U64 ticks)
{
    return static_cast<zb::U64>(static_cast<float>(ticks) / ticksPerSecond);
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline zb::U64 getDifficultyFactor(const zb::U64 ticks)
{
    constexpr zb::U64 baseDifficulty = 800u;
    return baseDifficulty + getElapsedSeconds(ticks) * 5u;
}


////////////////////////////////////////////////////////////
[[nodiscard]] zb::Array<zb::U64, 4> generateTetraminoHealthDistribution(zb::U64 difficultyFactor, auto&& rng)
{
    const auto minHealth = 1;
    const auto maxHealth = 4;

    // 1. Start with all blocks at minimum health.
    zb::Array<zb::U64, 4> healths{minHealth, minHealth, minHealth, minHealth};

    // 2. Iterate through each of the 4 block "slots" and attempt to upgrade them.
    for (zb::SizeT i = 0u; i < healths.size(); ++i)
    {
        // A block can try to upgrade multiple times in a row.
        while (healths[i] < maxHealth)
        {
            const zb::U64 targetHealth = healths[i] + 1;

            // --- Calculate the chance to upgrade to the targetHealth ---

            // a) Base chance decreases sharply for higher health values.
            //    Chance to get to HP 2 is 50%, HP 3 is 20%, HP 4 is 10%.
            float baseUpgradeChance = 0.f;
            switch (targetHealth)
            {
                case 2:
                    baseUpgradeChance = 0.2f;
                    break; // 50%
                case 3:
                    baseUpgradeChance = 0.4f;
                    break; // 20%
                case 4:
                    baseUpgradeChance = 0.15f;
                    break; // 10%
                case 5:
                    baseUpgradeChance = 0.15f;
                    break; // 10%
                default:
                    break;
            }

            // b) Difficulty modifier makes upgrades more likely over time.
            //    Adds up to 20% chance over ~16 mins of gameplay (difficulty 10k).
            const float difficultyModifier = (static_cast<float>(difficultyFactor) - 1775.f) / 5000.f;

            // c) Intra-tetramino penalty makes multiple high-HP blocks on one piece rare.
            //    Each existing block with >= targetHealth reduces the chance by 30%.
            int highHpBlocksCount = 0;
            for (zb::SizeT j = 0u; j < healths.size(); ++j)
            {
                if (i == j)
                    continue; // Don't count the block we are currently upgrading

                if (healths[j] > 1)
                    ++highHpBlocksCount;
            }

            const float penalty = static_cast<float>(highHpBlocksCount) * 0.4f; // 40% penalty per block

            // --- Final Calculation and Roll ---
            const float finalChance = (baseUpgradeChance + difficultyModifier) - penalty;

            if (rng.getF(0.f, 1.f) < finalChance)
            {
                // Success! Upgrade this block's health.
                healths[i]++;
            }
            else
            {
                // Failure. This block is done upgrading.
                break;
            }
        }
    }

    // 4. Shuffle the results so the high HP block isn't always in the first slot.
    shuffleBag(healths, rng);

    return healths;
}


////////////////////////////////////////////////////////////
struct [[nodiscard]] World
{
    TetraminoId nextTetraminoId{0u};
    BlockId     nextBlockId{0u};

    BlockGrid blockGrid{10u, 20u + gridGraceY};

    zb::Vector<TaggedBlockMatrix> blockMatrixBag;

    zb::Optional<Tetramino> currentTetramino;
    zb::Optional<Tetramino> heldTetramino;

    bool holdUsedThisTurn = false;

    zb::SizeT rerollsPerLevel      = 3u;
    zb::SizeT nPerkChoicesPerLevel = 3u;

    zb::U64 tick = 0u;

    zb::U64 dropTickAccumulator = 0u;
    zb::U64 dropTickTarget      = 60u;

    zb::U64 graceDropMoves      = 0u;
    zb::U64 maxGraceDropMoves   = 2u;
    bool    lastMoveWasRotation = false;

    zb::U32 playerLevel          = 1u;
    zb::U32 committedPlayerLevel = 1u;
    zb::U64 currentXP            = 0u;

    zb::U64 linesCleared    = 0u;
    zb::U64 tetaminosPlaced = 0u;

    int perkRndHitOnClear = 0;

    int perkCanHoldTetramino     = 0;
    int perkXPPerTetraminoPlaced = 0;
    int perkXPPerBlockDamaged    = 0;

    struct [[nodiscard]] DeleteFloorPerNTetraminos
    {
        int nTetraminos;
        int tetraminosPlacedCount = 0;
    };

    zb::Optional<DeleteFloorPerNTetraminos> perkDeleteFloorPerNTetraminos;

    int perkExtraLinePiecesInPool = 0;

    struct [[nodiscard]] DrillData
    {
        int coverage       = 1;
        int maxPenetration = 1;
    };

    zb::Optional<DrillData> perkDrill[drillDirectionCount]{
        // zb::Optional<DrillData>{DrillData{}},
        // zb::Optional<DrillData>{DrillData{}},
        // zb::Optional<DrillData>{DrillData{}},
    };

    int perkNPeek = 1;

    struct [[nodiscard]] RndHitPerNTetraminos
    {
        int nTetraminos;
        int tetraminosPlacedCount = 0;
    };

    zb::Optional<RndHitPerNTetraminos> perkRndHitPerNTetraminos;

    int perkChainLightning = 0;

    struct [[nodiscard]] LaserData
    {
        int  maxPenetration = 1;
        bool bounce         = false;
    };

    zb::Optional<LaserData> perkLaser[laserDirectionCount]{
        // zb::Optional<LaserData>{LaserData{}},
        // zb::Optional<LaserData>{LaserData{}},
    };
};

} // namespace tsurv
