#include "ZancleBase/MaxAlignT.hpp"

#include <cstddef>


static_assert(sizeof(zb::MaxAlignT) == sizeof(std::max_align_t));
static_assert(alignof(zb::MaxAlignT) == alignof(std::max_align_t));
