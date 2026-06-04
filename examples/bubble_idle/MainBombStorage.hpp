#pragma once

#include "ZancleBase/AnkerlUnorderedDense.hpp"
#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
struct MainBombStorage
{
    ankerl::unordered_dense::map<zb::SizeT, zb::SizeT> bombIdxToCatIdx;
};
