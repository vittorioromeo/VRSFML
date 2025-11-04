#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "DrillDirection.hpp"
#include "LaserDirection.hpp"
#include "Perk.hpp"
#include "World.hpp"

#include <format>
#include <string>


namespace tsurv
{
/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkChainLightning : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int chanceIncrease = 10;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "Chain Lightning";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        return std::format("Each time lightning strikes, add a {}% chance to hit an additional block.",
                           world.perkChainLightning + chanceIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{}% -> {}%", world.perkChainLightning, world.perkChainLightning + chanceIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("({}% chance)", world.perkChainLightning);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkChainLightning > 0;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return (world.perkRndHitPerNTetraminos.hasValue() || world.perkRndHitOnClear > 0) && world.perkChainLightning < 60;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkChainLightning += 10;
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkPeekNextTetraminos : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int peekIncrease = 1;
    static constexpr int maxPeek      = 3;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "Peek Next Tetraminos";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        return std::format("See the next {} upcoming tetraminos.", world.perkNPeek + peekIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}", world.perkNPeek, world.perkNPeek + peekIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("(see {} tetraminos)", world.perkNPeek);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkNPeek > 1;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkNPeek < maxPeek;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkNPeek += peekIncrease;
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkOnClearLightningStrike : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int strikeIncrease = 1;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "On-Clear Lightning Strike";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        return std::
            format("Each time you full-clear a line or more, randomly damage {} block(s) with a lightning strike.",
                   world.perkRndHitOnClear + strikeIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}", world.perkRndHitOnClear, world.perkRndHitOnClear + strikeIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("({} tetraminos per clear)", world.perkRndHitOnClear);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World&) const override
    {
        return true; // Always available to upgrade
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkRndHitOnClear > 0;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkRndHitOnClear += strikeIncrease;
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkVerticalDrillUnlock : Perk
{
    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "Vertical Drill";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World&) const override
    {
        return "When hard dropping, automatically damage blocks directly connected below the tetramino's sharp edges.";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World&) const override
    {
        return ""; // One-time unlock
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("(penetration: {}, coverage: {})",
                           world.perkDrill[DrillDirection::Down]->maxPenetration,
                           world.perkDrill[DrillDirection::Down]->coverage);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkDrill[DrillDirection::Down].hasValue();
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return !isActive(world);
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkDrill[DrillDirection::Down].emplace();
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkVerticalDrillPenetration : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int penetrationIncrease = 1;
    static constexpr int maxPenetration      = 10;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "Hard-Drop Drill - Penetration";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        return std::format("Increase the maximum number of blocks damaged to {}.",
                           world.perkDrill[DrillDirection::Down]->maxPenetration + penetrationIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}",
                           world.perkDrill[DrillDirection::Down]->maxPenetration,
                           world.perkDrill[DrillDirection::Down]->maxPenetration + penetrationIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World&) const override
    {
        return ""; // handled by main perk
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkDrill[DrillDirection::Down].hasValue() &&
               world.perkDrill[DrillDirection::Down]->maxPenetration > 1;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkDrill[DrillDirection::Down].hasValue() &&
               world.perkDrill[DrillDirection::Down]->maxPenetration < maxPenetration;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkDrill[DrillDirection::Down]->maxPenetration += penetrationIncrease;
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkVerticalDrillBluntForce : Perk
{
    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "Hard-Drop Drill - Blunt Force";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World&) const override
    {
        return "The entire surface of the tetramino acts as a drill when hard-dropping.";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World&) const override
    {
        return ""; // One-time unlock
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World&) const override
    {
        return ""; // handled by main perk
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkDrill[DrillDirection::Down].hasValue() && world.perkDrill[DrillDirection::Down]->coverage == 2;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkDrill[DrillDirection::Down].hasValue() && world.perkDrill[DrillDirection::Down]->coverage == 1;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkDrill[DrillDirection::Down]->coverage = 2; // special case value meaning "full coverage"
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkHoldSkipTetramino : Perk
{
    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        // This perk is a multi-stage upgrade. We assume it is only shown when available.
        // The first time (world.perkCanHoldTetramino == 0) it unlocks "Hold".
        // The second time (world.perkCanHoldTetramino == 1) it upgrades "Hold" to "Skip".
        return "Hold / Skip Tetramino";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        if (world.perkCanHoldTetramino == 0)
            return "Gain the ability to hold your current tetramino. Can be upgraded to skip a tetramino later.";

        return "Upgrade 'Hold' to 'Skip', allowing you to discard your current tetramino once per turn.";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        if (world.perkCanHoldTetramino == 0)
            return "Unlock Hold";

        if (world.perkCanHoldTetramino == 1)
            return "Hold -> Skip";

        return "";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("{}", (world.perkCanHoldTetramino == 0) ? "Hold" : "Skip");
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkCanHoldTetramino > 0;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkCanHoldTetramino < 2;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        ++world.perkCanHoldTetramino;
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkXpPerTetraminoPlaced : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int xpIncrease = 3;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "XP per Tetramino Placed";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        /////////////////////////////////////////////////////////////
        return std::format("Gain {} XP for each tetramino you place.", world.perkXPPerTetraminoPlaced + xpIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}", world.perkXPPerTetraminoPlaced, world.perkXPPerTetraminoPlaced + xpIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("({} XP)", world.perkXPPerTetraminoPlaced);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkXPPerTetraminoPlaced > 0;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World&) const override
    {
        return true;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkXPPerTetraminoPlaced += xpIncrease;
    }
};

/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkXpPerBlockDamaged : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int xpIncrease = 10;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "XP per Block Damaged";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        /////////////////////////////////////////////////////////////
        return std::format("Gain {} XP for each block you damage.", world.perkXPPerBlockDamaged + xpIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}", world.perkXPPerBlockDamaged, world.perkXPPerBlockDamaged + xpIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("({} XP)", world.perkXPPerBlockDamaged);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkXPPerBlockDamaged > 0;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World&) const override
    {
        return true;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkXPPerBlockDamaged += xpIncrease;
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkDeleteFloorPerNTetraminos : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int initialThreshold = 25;
    static constexpr int minThreshold     = 10;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "Janitor";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        if (!world.perkDeleteFloorPerNTetraminos.hasValue())
            return std::format("Every {} tetraminos you place, delete the bottom row of the grid.", initialThreshold);

        return "Decrease the number of tetraminos needed to trigger the Janitor effect.";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        if (!world.perkDeleteFloorPerNTetraminos.hasValue())
            return std::format("Activates at {}", initialThreshold);

        return std::format("{} -> {}",
                           world.perkDeleteFloorPerNTetraminos->nTetraminos,
                           world.perkDeleteFloorPerNTetraminos->nTetraminos - 1);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("(every {} tetraminos)", world.perkDeleteFloorPerNTetraminos->nTetraminos);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkDeleteFloorPerNTetraminos.hasValue();
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return !world.perkDeleteFloorPerNTetraminos.hasValue() ||
               world.perkDeleteFloorPerNTetraminos->nTetraminos > minThreshold;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        if (!world.perkDeleteFloorPerNTetraminos.hasValue())
            world.perkDeleteFloorPerNTetraminos.emplace(initialThreshold);
        else
            --world.perkDeleteFloorPerNTetraminos->nTetraminos;
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkRndHitPerNTetraminos : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int initialThreshold = 15;
    static constexpr int minThreshold     = 8;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "On-Placement Strike";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        if (!world.perkRndHitPerNTetraminos.hasValue())
            return std::format("Every {} tetraminos you place, randomly damage a block with a lightning strike.",
                               initialThreshold);

        return "Decrease the number of tetraminos needed to trigger the On-Placement Strike.";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        if (!world.perkRndHitPerNTetraminos.hasValue())
            return std::format("Activates at {}", initialThreshold);

        return std::format("{} -> {}",
                           world.perkRndHitPerNTetraminos->nTetraminos,
                           world.perkRndHitPerNTetraminos->nTetraminos - 1);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("(every {} tetraminos)", world.perkRndHitPerNTetraminos->nTetraminos);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkRndHitPerNTetraminos.hasValue();
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return !world.perkRndHitPerNTetraminos.hasValue() || world.perkRndHitPerNTetraminos->nTetraminos > minThreshold;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        if (!world.perkRndHitPerNTetraminos.hasValue())
            world.perkRndHitPerNTetraminos.emplace(initialThreshold);
        else
            --world.perkRndHitPerNTetraminos->nTetraminos;
    }
};


/////////////////////////////////////////////////////////////
struct [[nodiscard]] PerkExtraLinePieces : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int pieceIncrease  = 1;
    static constexpr int maxExtraPieces = 3;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        return "Extra Line Pieces";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World&) const override
    {
        return std::format("Increase the number of line pieces in the tetramino bag by {}.", pieceIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}", world.perkExtraLinePiecesInPool, world.perkExtraLinePiecesInPool + pieceIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("(+{})", world.perkExtraLinePiecesInPool);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkExtraLinePiecesInPool > 0;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkExtraLinePiecesInPool < maxExtraPieces;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkExtraLinePiecesInPool += pieceIncrease;
    }
};


/////////////////////////////////////////////////////////////
template <DrillDirection::Enum TDirection>
struct [[nodiscard]] PerkHorizontalDrillUnlock : Perk
{
    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        const auto* upperCaseDirectionStr = (TDirection == DrillDirection::Left) ? "Left" : "Right";
        return std::format("{} Horizontal Drill", upperCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World&) const override
    {
        const auto* lowerCaseDirectionStr = (TDirection == DrillDirection::Left) ? "left" : "right";
        return std::format("Damage blocks directly adjacent to the {} of the placed tetramino.", lowerCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World&) const override
    {
        return "";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("(penetration: {}, coverage: {})",
                           world.perkDrill[TDirection]->maxPenetration,
                           world.perkDrill[TDirection]->coverage);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkDrill[TDirection].hasValue();
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return !world.perkDrill[TDirection].hasValue();
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkDrill[TDirection].emplace();
    }
};


/////////////////////////////////////////////////////////////
template <DrillDirection::Enum TDirection>
struct [[nodiscard]] PerkHorizontalDrillPenetration : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int penetrationIncrease = 1;
    static constexpr int maxPenetration      = 4;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        const auto* upperCaseDirectionStr = (TDirection == DrillDirection::Left) ? "Left" : "Right";
        return std::format("{} Drill - Penetration", upperCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        return std::format("Increase the maximum number of blocks damaged to {}.",
                           world.perkDrill[TDirection]->maxPenetration + penetrationIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}",
                           world.perkDrill[TDirection]->maxPenetration,
                           world.perkDrill[TDirection]->maxPenetration + penetrationIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World&) const override
    {
        return ""; // handled by main perk
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkDrill[TDirection].hasValue() && world.perkDrill[TDirection]->maxPenetration > 1;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkDrill[TDirection].hasValue() && world.perkDrill[TDirection]->maxPenetration < maxPenetration;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        ++world.perkDrill[TDirection]->maxPenetration;
    }
};


/////////////////////////////////////////////////////////////
template <DrillDirection::Enum TDirection>
struct [[nodiscard]] PerkHorizontalDrillCoverage : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int lengthIncrease = 1;
    static constexpr int maxLength      = 4;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        const auto* upperCaseDirectionStr = (TDirection == DrillDirection::Left) ? "Left" : "Right";
        return std::format("{} Drill - Coverage", upperCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        return std::format("Increase the maximum length of the drill to {} blocks.",
                           world.perkDrill[TDirection]->coverage + lengthIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}",
                           world.perkDrill[TDirection]->coverage,
                           world.perkDrill[TDirection]->coverage + lengthIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World&) const override
    {
        return ""; // handled by main perk
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkDrill[TDirection].hasValue() && world.perkDrill[TDirection]->coverage > 1;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkDrill[TDirection].hasValue() && world.perkDrill[TDirection]->coverage < maxLength;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        ++world.perkDrill[TDirection]->coverage;
    }
};


/////////////////////////////////////////////////////////////
using PerkHorizontalDrillLeftUnlock  = PerkHorizontalDrillUnlock<DrillDirection::Left>;
using PerkHorizontalDrillRightUnlock = PerkHorizontalDrillUnlock<DrillDirection::Right>;


/////////////////////////////////////////////////////////////
using PerkHorizontalDrillLeftPenetration  = PerkHorizontalDrillPenetration<DrillDirection::Left>;
using PerkHorizontalDrillRightPenetration = PerkHorizontalDrillPenetration<DrillDirection::Right>;


/////////////////////////////////////////////////////////////
using PerkHorizontalDrillLeftCoverage  = PerkHorizontalDrillCoverage<DrillDirection::Left>;
using PerkHorizontalDrillRightCoverage = PerkHorizontalDrillCoverage<DrillDirection::Right>;


/////////////////////////////////////////////////////////////
template <LaserDirection::Enum TDirection>
struct [[nodiscard]] PerkDiagonalLaserUnlock : Perk
{
    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        const auto* upperCaseDirectionStr = (TDirection == LaserDirection::Left) ? "SW" : "SE";
        return std::format("{} Diagonal Laser", upperCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World&) const override
    {
        const auto* lowerCaseDirectionStr = (TDirection == LaserDirection::Left) ? "southwest" : "southeast";
        return std::format("Damage blocks placed diagonally to the {} of the placed tetramino.", lowerCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World&) const override
    {
        return "";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World& world) const override
    {
        return std::format("(penetration: {}, bounce: {})",
                           world.perkLaser[TDirection]->maxPenetration,
                           world.perkLaser[TDirection]->bounce ? "yes" : "no");
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkLaser[TDirection].hasValue();
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return !isActive(world);
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkLaser[TDirection].emplace();
    }
};


/////////////////////////////////////////////////////////////
template <LaserDirection::Enum TDirection>
struct [[nodiscard]] PerkDiagonalLaserPenetration : Perk
{
    /////////////////////////////////////////////////////////////
    static constexpr int penetrationIncrease = 1;
    static constexpr int maxPenetration      = 4;

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        const auto* upperCaseDirectionStr = (TDirection == LaserDirection::Left) ? "SW" : "SE";
        return std::format("{} Diagonal - Penetration", upperCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World& world) const override
    {
        return std::format("Increase the maximum number of blocks damaged to {}.",
                           world.perkLaser[TDirection]->maxPenetration + penetrationIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World& world) const override
    {
        return std::format("{} -> {}",
                           world.perkLaser[TDirection]->maxPenetration,
                           world.perkLaser[TDirection]->maxPenetration + penetrationIncrease);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World&) const override
    {
        return ""; // handled by main perk
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkLaser[TDirection].hasValue() && world.perkLaser[TDirection]->maxPenetration > 1;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkLaser[TDirection].hasValue() && world.perkLaser[TDirection]->maxPenetration < maxPenetration;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        ++world.perkLaser[TDirection]->maxPenetration;
    }
};


/////////////////////////////////////////////////////////////
template <LaserDirection::Enum TDirection>
struct [[nodiscard]] PerkDiagonalLaserBounce : Perk
{
    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getName() const override
    {
        const auto* upperCaseDirectionStr = (TDirection == LaserDirection::Left) ? "SW" : "SE";
        return std::format("{} Diagonal - Bounce", upperCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getDescription(const World&) const override
    {
        const auto* lowerCaseDirectionStr = (TDirection == LaserDirection::Left) ? "southwest" : "southeast";

        return std::format("Lasers fired {} will now bounce off the sides of the grid once.", lowerCaseDirectionStr);
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getProgressionStr(const World&) const override
    {
        return "";
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string getInventoryStr(const World&) const override
    {
        return ""; // handled by main perk
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool isActive(const World& world) const override
    {
        return world.perkLaser[TDirection].hasValue() && world.perkLaser[TDirection]->bounce;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool meetsPrerequisites(const World& world) const override
    {
        return world.perkLaser[TDirection].hasValue() && !world.perkLaser[TDirection]->bounce;
    }

    /////////////////////////////////////////////////////////////
    void apply(World& world) const override
    {
        world.perkLaser[TDirection]->bounce = true;
    }
};


/////////////////////////////////////////////////////////////
using PerkDiagonalLaserLeftUnlock  = PerkDiagonalLaserUnlock<LaserDirection::Left>;
using PerkDiagonalLaserRightUnlock = PerkDiagonalLaserUnlock<LaserDirection::Right>;


/////////////////////////////////////////////////////////////
using PerkDiagonalLaserLeftPenetration  = PerkDiagonalLaserPenetration<LaserDirection::Left>;
using PerkDiagonalLaserRightPenetration = PerkDiagonalLaserPenetration<LaserDirection::Right>;


/////////////////////////////////////////////////////////////
using PerkDiagonalLaserLeftBounce  = PerkDiagonalLaserBounce<LaserDirection::Left>;
using PerkDiagonalLaserRightBounce = PerkDiagonalLaserBounce<LaserDirection::Right>;

} // namespace tsurv
