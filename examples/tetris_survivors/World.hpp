#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "BlockGrid.hpp"
#include "BlockMatrix.hpp"
#include "Constants.hpp"
#include "Tetramino.hpp"

#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] TaggedBlockMatrix // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    BlockMatrix   blockMatrix;
    TetraminoType tetraminoType;
};


////////////////////////////////////////////////////////////
[[nodiscard]] inline sf::base::U64 getXPNeededForLevelUp(const sf::base::U32 level)
{
    constexpr double baseXP   = 30.0;
    constexpr double exponent = 1.075;

    return static_cast<sf::base::U64>(baseXP * sf::base::pow(static_cast<double>(level), exponent));
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline sf::base::U64 getElapsedSeconds(const sf::base::U64 ticks)
{
    return static_cast<sf::base::U64>(static_cast<float>(ticks) / ticksPerSecond);
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline sf::base::U64 getDifficultyFactor(const sf::base::U64 ticks)
{
    constexpr sf::base::U64 baseDifficulty = 1000u;
    return baseDifficulty + getElapsedSeconds(ticks) * 5u;
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::Array<sf::base::U64, 4> generateTetraminoHealthDistribution(sf::base::U64 difficultyFactor, auto&& rng)
{
    const auto minHealth = 1;
    const auto maxHealth = 4;

    // 1. Start with all blocks at minimum health.
    sf::base::Array<sf::base::U64, 4> healths{minHealth, minHealth, minHealth, minHealth};

    // 2. Iterate through each of the 4 block "slots" and attempt to upgrade them.
    for (sf::base::SizeT i = 0u; i < healths.size(); ++i)
    {
        // A block can try to upgrade multiple times in a row.
        while (healths[i] < maxHealth)
        {
            const sf::base::U64 targetHealth = healths[i] + 1;

            // --- Calculate the chance to upgrade to the targetHealth ---

            // a) Base chance decreases sharply for higher health values.
            //    Chance to get to HP 2 is 50%, HP 3 is 20%, HP 4 is 10%.
            float baseUpgradeChance = 0.f;
            switch (targetHealth)
            {
                case 2:
                    baseUpgradeChance = 0.15f;
                    break; // 50%
                case 3:
                    baseUpgradeChance = 0.15f;
                    break; // 20%
                case 4:
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
            for (sf::base::SizeT j = 0u; j < healths.size(); ++j)
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
    sf::base::U32 nextTetraminoId = 0u;
    sf::base::U32 nextBlockId     = 0u;

    BlockGrid blockGrid{10u, 20u + gridGraceY};

    sf::base::Vector<TaggedBlockMatrix> blockMatrixBag;

    sf::base::Optional<Tetramino> currentTetramino;
    sf::base::Optional<Tetramino> heldTetramino;

    bool holdUsedThisTurn = false;

    sf::base::U64 tick              = 0u;
    sf::base::U64 graceDropMoves    = 0u;
    sf::base::U64 maxGraceDropMoves = 2u;

    sf::base::U32 playerLevel          = 1u;
    sf::base::U32 committedPlayerLevel = 1u;
    sf::base::U64 currentXP            = 0u;

    sf::base::U64 linesCleared    = 0u;
    sf::base::U64 tetaminosPlaced = 0u;

    int perkRndHitOnClear = 0;

    struct VerticalDrill
    {
        int  maxPenetration = 1;
        bool multiHit       = false;
    };

    sf::base::Optional<VerticalDrill> perkVerticalDrill;

    int perkCanHoldTetramino     = 0;
    int perkXPPerTetraminoPlaced = 0;
    int perkXPPerBlockDamaged    = 0;

    struct DeleteFloorPerNTetraminos
    {
        int nTetraminos;
        int tetraminosPlacedCount = 0;
    };

    sf::base::Optional<DeleteFloorPerNTetraminos> perkDeleteFloorPerNTetraminos;

    int perkExtraLinePiecesInPool = 0;

    struct HorizontalDrill
    {
        int maxBlocks      = 1;
        int maxPenetration = 1;
    };

    sf::base::Optional<HorizontalDrill> perkHorizontalDrillLeft;
    sf::base::Optional<HorizontalDrill> perkHorizontalDrillRight;

    int perkNPeek = 1;

    struct RndHitPerNTetraminos
    {
        int nTetraminos;
        int tetraminosPlacedCount = 0;
    };

    sf::base::Optional<RndHitPerNTetraminos> perkRndHitPerNTetraminos;

    int perkChainLightning = 0;
};

} // namespace tsurv
