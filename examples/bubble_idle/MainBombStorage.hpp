#pragma once

#include "Zancle/Container/AnkerlUnorderedDense.hpp"

#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct MainBombStorage
{
    ankerl::unordered_dense::map<za::SizeT, za::SizeT> bombIdxToCatIdx;
};
