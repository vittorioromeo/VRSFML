#include "Zancle/Base/PtrDiffT.hpp"

#include <cstddef>


////////////////////////////////////////////////////////////
static_assert(sizeof(za::PtrDiffT) == sizeof(std::ptrdiff_t));
static_assert(alignof(za::PtrDiffT) == alignof(std::ptrdiff_t));
