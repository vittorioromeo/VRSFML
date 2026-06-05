#include "Zancle/Base/UIntPtrT.hpp"

#include <cstdint>


////////////////////////////////////////////////////////////
static_assert(sizeof(za::UIntPtrT) == sizeof(std::uintptr_t));
static_assert(alignof(za::UIntPtrT) == alignof(std::uintptr_t));
