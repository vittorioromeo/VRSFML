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

#include "Zancle/Container/Array.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Math/Pow.hpp"
#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Container/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] TaggedBlockMatrix // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    BlockMatrix   blockMatrix;
    TetraminoType tetraminoType;
};


////////////////////////////////////////////////////////////
[[nodiscard]] inline za::U64 getXPNeededForLevelUp(const za::U32 level)
{
    constexpr double baseXP   = 30.0;
    constexpr double exponent = 1.035;

    return static_cast<za::U64>(baseXP * za::pow(static_cast<double>(level), exponent));
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline za::U64 getElapsedSeconds(const za::U64 ticks)
{
    return static_cast<za::U64>(static_cast<float>(ticks) / ticksPerSecond);
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline za::U64 getDifficultyFactor(const za::U64 ticks)
{
    constexpr za::U64 baseDifficulty = 800u;
    return baseDifficulty + getElapsedSeconds(ticks) * 5u;
}


////////////////////////////////////////////////////////////
[[nodiscard]] za::Array<za::U64, 4> generateTetraminoHealthDistribution(za::U64 difficultyFactor, auto&& rng)
{
    const auto minHealth = 1;
    const auto maxHealth = 4;

    za::Array<za::U64, 4> healths{minHealth, minHealth, minHealth, minHealth};

    for (za::SizeT i = 0u; i < healths.size(); ++i)
    {
        // A block can try to upgrade multiple times in a row.
        while (healths[i] < maxHealth)
        {
            const za::U64 targetHealth = healths[i] + 1;

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
            for (za::SizeT j = 0u; j < healths.size(); ++j)
            {
                if (i == j)
                    continue; // Don't count the block we are currently upgrading

                if (healths[j] > 1)
                    ++highHpBlocksCount;
            }

            const float penalty = static_cast<float>(highHpBlocksCount) * 0.4f; // 40% penalty per block

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

    shuffleBag(healths, rng);

    return healths;
}


////////////////////////////////////////////////////////////
struct [[nodiscard]] World
{
    TetraminoId nextTetraminoId{0u};
    BlockId     nextBlockId{0u};

    BlockGrid blockGrid{10u, 20u + gridGraceY};

    za::Vector<TaggedBlockMatrix> blockMatrixBag;

    za::Optional<Tetramino> currentTetramino;
    za::Optional<Tetramino> heldTetramino;

    bool holdUsedThisTurn = false;

    za::SizeT rerollsPerLevel      = 3u;
    za::SizeT nPerkChoicesPerLevel = 3u;

    za::U64 tick = 0u;

    za::U64 dropTickAccumulator = 0u;
    za::U64 dropTickTarget      = 60u;

    za::U64 graceDropMoves      = 0u;
    za::U64 maxGraceDropMoves   = 2u;
    bool    lastMoveWasRotation = false;

    za::U32 playerLevel          = 1u;
    za::U32 committedPlayerLevel = 1u;
    za::U64 currentXP            = 0u;

    za::U64 linesCleared    = 0u;
    za::U64 tetaminosPlaced = 0u;

    int perkRndHitOnClear = 0;

    int perkCanHoldTetramino     = 0;
    int perkXPPerTetraminoPlaced = 0;
    int perkXPPerBlockDamaged    = 0;

    struct [[nodiscard]] DeleteFloorPerNTetraminos
    {
        int nTetraminos;
        int tetraminosPlacedCount = 0;
    };

    za::Optional<DeleteFloorPerNTetraminos> perkDeleteFloorPerNTetraminos;

    int perkExtraLinePiecesInPool = 0;

    struct [[nodiscard]] DrillData
    {
        int coverage       = 1;
        int maxPenetration = 1;
    };

    za::Optional<DrillData> perkDrill[drillDirectionCount]{
        // za::Optional<DrillData>{DrillData{}},
        // za::Optional<DrillData>{DrillData{}},
        // za::Optional<DrillData>{DrillData{}},
    };

    int perkNPeek = 1;

    struct [[nodiscard]] RndHitPerNTetraminos
    {
        int nTetraminos;
        int tetraminosPlacedCount = 0;
    };

    za::Optional<RndHitPerNTetraminos> perkRndHitPerNTetraminos;

    int perkChainLightning = 0;

    struct [[nodiscard]] LaserData
    {
        int  maxPenetration = 1;
        bool bounce         = false;
    };

    za::Optional<LaserData> perkLaser[laserDirectionCount]{
        // za::Optional<LaserData>{LaserData{}},
        // za::Optional<LaserData>{LaserData{}},
    };
};

} // namespace tsurv
