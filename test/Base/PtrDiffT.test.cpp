#include "ZancleBase/PtrDiffT.hpp"

#include <cstddef>


////////////////////////////////////////////////////////////
static_assert(sizeof(zb::PtrDiffT) == sizeof(std::ptrdiff_t));
static_assert(alignof(zb::PtrDiffT) == alignof(std::ptrdiff_t));
