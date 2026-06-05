#include "Zancle/Base/SizeT.hpp"

#include <cstddef>


static_assert(sizeof(za::SizeT) == sizeof(std::size_t));
static_assert(alignof(za::SizeT) == alignof(std::size_t));
