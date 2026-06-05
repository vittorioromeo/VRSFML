#pragma once

#include "GrowthFactors.hpp"

#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] PSVData
{
    const za::SizeT     nMaxPurchases;
    const GrowthFactors cost;
    const GrowthFactors value;
};
