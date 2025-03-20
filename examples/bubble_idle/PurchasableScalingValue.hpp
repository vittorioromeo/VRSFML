#pragma once

#include "Aliases.hpp"
#include "PSVData.hpp"

#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] PurchasableScalingValue
{
    const PSVData*  data; // TODO P2: eww
    sf::base::SizeT nPurchases = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr float costAt(const sf::base::SizeT n) const
    {
        return data->cost.computeGrowth(static_cast<float>(n));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr float valueAt(const sf::base::SizeT n) const
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
    [[nodiscard, gnu::always_inline]] inline constexpr float cumulativeCostBetween(
        const sf::base::SizeT nStart /* inclusive */,
        const sf::base::SizeT nEnd /* exclusive */) const
    {
        float totalCost = 0.f;

        for (sf::base::SizeT i = nStart; i < nEnd; ++i)
            totalCost += data->cost.computeGrowth(static_cast<float>(i));

        return totalCost;
    }
};
