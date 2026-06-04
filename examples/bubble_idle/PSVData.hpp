#pragma once

#include "GrowthFactors.hpp"

#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] PSVData
{
    const zb::SizeT     nMaxPurchases;
    const GrowthFactors cost;
    const GrowthFactors value;
};
