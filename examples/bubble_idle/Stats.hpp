#pragma once

#include "BubbleType.hpp"
#include "CatType.hpp"
#include "PSVDataConstants.hpp"

#include "ZancleBase/IntTypes.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Stats
{
    zb::U64 secondsPlayed = 0u;

    zb::U64 nBubblesPoppedByType[nBubbleTypes] = {};
    zb::U64 revenueByType[nBubbleTypes]        = {};

    zb::U64 nBubblesHandPoppedByType[nBubbleTypes] = {};
    zb::U64 revenueHandByType[nBubbleTypes]        = {};

    zb::U64 explosionRevenue  = 0u;
    zb::U64 flightRevenue     = 0u;
    zb::U64 hellPortalRevenue = 0u;

    zb::U64 highestStarBubblePopCombo = 0u;
    zb::U64 highestNovaBubblePopCombo = 0u;

    zb::U64 nAbsorbedStarBubbles = 0u;

    zb::U64 nSpellCasts[PSVDataConstants::spellCount.nMaxPurchases] = {};

    zb::U64 nWitchcatRitualsPerCatType[nCatTypes] = {};
    zb::U64 nWitchcatDollsCollected               = 0u;

    zb::U64 nMaintenances                   = 0u;
    zb::U64 highestSimultaneousMaintenances = 0u;

    zb::U64 nDisguises = 0u;

    zb::U64 highestDPS = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getTotalNBubblesPopped() const noexcept
    {
        zb::U64 acc = 0u;

        for (const auto x : nBubblesPoppedByType)
            acc += x;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getTotalRevenue() const noexcept
    {
        zb::U64 acc = 0u;

        for (const auto x : revenueByType)
            acc += x;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getTotalNBubblesHandPopped() const noexcept
    {
        zb::U64 acc = 0u;

        for (const auto x : nBubblesHandPoppedByType)
            acc += x;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getTotalRevenueHand() const noexcept
    {
        zb::U64 acc = 0u;

        for (const auto x : revenueHandByType)
            acc += x;

        return acc;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getTotalNBubblesCatPopped() const noexcept
    {
        return getTotalNBubblesPopped() - getTotalNBubblesHandPopped();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getTotalRevenueCat() const noexcept
    {
        return getTotalRevenue() - getTotalRevenueHand();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getNBubblesPopped(const BubbleType bubbleType) const noexcept
    {
        return nBubblesPoppedByType[asIdx(bubbleType)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getRevenue(const BubbleType bubbleType) const noexcept
    {
        return revenueByType[asIdx(bubbleType)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getNBubblesHandPopped(const BubbleType bubbleType) const noexcept
    {
        return nBubblesHandPoppedByType[asIdx(bubbleType)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getRevenueHand(const BubbleType bubbleType) const noexcept
    {
        return revenueHandByType[asIdx(bubbleType)];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getNBubblesCatPopped(const BubbleType bubbleType) const noexcept
    {
        return getNBubblesPopped(bubbleType) - getNBubblesHandPopped(bubbleType);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] zb::U64 getRevenueCat(const BubbleType bubbleType) const noexcept
    {
        return getRevenue(bubbleType) - getRevenueHand(bubbleType);
    }
};
