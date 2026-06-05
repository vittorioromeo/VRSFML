#pragma once

#include "BubbleType.hpp"
#include "CatType.hpp"
#include "PSVDataConstants.hpp"

#include "Zancle/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Stats
{
    za::U64 secondsPlayed = 0u;

    za::U64 nBubblesPoppedByType[nBubbleTypes] = {};
    za::U64 revenueByType[nBubbleTypes]        = {};

    za::U64 nBubblesHandPoppedByType[nBubbleTypes] = {};
    za::U64 revenueHandByType[nBubbleTypes]        = {};

    za::U64 explosionRevenue  = 0u;
    za::U64 flightRevenue     = 0u;
    za::U64 hellPortalRevenue = 0u;

    za::U64 highestStarBubblePopCombo = 0u;
    za::U64 highestNovaBubblePopCombo = 0u;

    za::U64 nAbsorbedStarBubbles = 0u;

    za::U64 nSpellCasts[PSVDataConstants::spellCount.nMaxPurchases] = {};

    za::U64 nWitchcatRitualsPerCatType[nCatTypes] = {};
    za::U64 nWitchcatDollsCollected               = 0u;

    za::U64 nMaintenances                   = 0u;
    za::U64 highestSimultaneousMaintenances = 0u;

    za::U64 nDisguises = 0u;

    za::U64 highestDPS = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getTotalNBubblesPopped() const noexcept
    {
        za::U64 acc = 0u;

        for (const auto x : nBubblesPoppedByType)
            acc += x;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getTotalRevenue() const noexcept
    {
        za::U64 acc = 0u;

        for (const auto x : revenueByType)
            acc += x;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getTotalNBubblesHandPopped() const noexcept
    {
        za::U64 acc = 0u;

        for (const auto x : nBubblesHandPoppedByType)
            acc += x;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getTotalRevenueHand() const noexcept
    {
        za::U64 acc = 0u;

        for (const auto x : revenueHandByType)
            acc += x;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getTotalNBubblesCatPopped() const noexcept
    {
        return getTotalNBubblesPopped() - getTotalNBubblesHandPopped();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getTotalRevenueCat() const noexcept
    {
        return getTotalRevenue() - getTotalRevenueHand();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getNBubblesPopped(const BubbleType bubbleType) const noexcept
    {
        return nBubblesPoppedByType[asIdx(bubbleType)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getRevenue(const BubbleType bubbleType) const noexcept
    {
        return revenueByType[asIdx(bubbleType)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getNBubblesHandPopped(const BubbleType bubbleType) const noexcept
    {
        return nBubblesHandPoppedByType[asIdx(bubbleType)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getRevenueHand(const BubbleType bubbleType) const noexcept
    {
        return revenueHandByType[asIdx(bubbleType)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getNBubblesCatPopped(const BubbleType bubbleType) const noexcept
    {
        return getNBubblesPopped(bubbleType) - getNBubblesHandPopped(bubbleType);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] za::U64 getRevenueCat(const BubbleType bubbleType) const noexcept
    {
        return getRevenue(bubbleType) - getRevenueHand(bubbleType);
    }
};
