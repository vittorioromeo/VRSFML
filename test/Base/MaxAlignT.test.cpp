#include "Zancle/Base/MaxAlignT.hpp"

#include <cstddef>


static_assert(sizeof(za::MaxAlignT) == sizeof(std::max_align_t));
static_assert(alignof(za::MaxAlignT) == alignof(std::max_align_t));
