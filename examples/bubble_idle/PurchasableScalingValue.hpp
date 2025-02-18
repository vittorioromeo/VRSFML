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
    [[nodiscard, gnu::always_inline]] inline constexpr float nextCost() const
    {
        return data->cost.computeGrowth(static_cast<float>(nPurchases));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline constexpr float currentValue() const
    {
        return data->value.computeGrowth(static_cast<float>(nPurchases));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr auto maxSubsequentPurchases(const MoneyType money) const
    {
        struct Result
        {
            sf::base::SizeT times    = 0u;
            MoneyType       maxCost  = 0u;
            MoneyType       nextCost = 0u;
        } result;

        MoneyType       cumulative = 0;
        sf::base::SizeT i          = nPurchases;

        // Try to purchase as many subsequent upgrades as possible.
        for (; i < data->nMaxPurchases; ++i)
        {
            const auto currentCost = static_cast<MoneyType>(data->cost.computeGrowth(static_cast<float>(i)));

            if (cumulative + currentCost > money)
                break;

            cumulative += currentCost;
            ++result.times;
            result.maxCost = cumulative;
        }

        // If further purchases are possible, compute the cost for one more.
        if (nPurchases < data->nMaxPurchases)
            result.nextCost = cumulative + static_cast<MoneyType>(data->cost.computeGrowth(static_cast<float>(i)));

        return result;
    }
};
