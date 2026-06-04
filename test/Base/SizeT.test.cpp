#include "ZancleBase/SizeT.hpp"

#include <cstddef>


static_assert(sizeof(zb::SizeT) == sizeof(std::size_t));
static_assert(alignof(zb::SizeT) == alignof(std::size_t));
