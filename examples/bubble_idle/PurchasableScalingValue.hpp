#pragma once

#include "PSVData.hpp"

#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] PurchasableScalingValue
{
    const PSVData* data; // TODO P2: eww
    za::SizeT      nPurchases = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr float costAt(const za::SizeT n) const
    {
        return data->cost.computeGrowth(static_cast<float>(n));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr float valueAt(const za::SizeT n) const
    {
        return data->value.computeGrowth(static_cast<float>(n));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr float nextCost() const
    {
        return costAt(nPurchases);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr float currentValue() const
    {
        return valueAt(nPurchases);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr float nextValue() const
    {
        return valueAt(nPurchases + 1u);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline constexpr float cumulativeCostBetween(const za::SizeT nStart /* inclusive */,
                                                                                   const za::SizeT nEnd /* exclusive */) const
    {
        float totalCost = 0.f;

        for (za::SizeT i = nStart; i < nEnd; ++i)
            totalCost += data->cost.computeGrowth(static_cast<float>(i));

        return totalCost;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr bool isMaxedOut() const
    {
        return nPurchases >= data->nMaxPurchases;
    }
};
