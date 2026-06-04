#include "ZancleBase/UIntPtrT.hpp"

#include <cstdint>


////////////////////////////////////////////////////////////
static_assert(sizeof(zb::UIntPtrT) == sizeof(std::uintptr_t));
static_assert(alignof(zb::UIntPtrT) == alignof(std::uintptr_t));
