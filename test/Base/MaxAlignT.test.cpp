#include <SFML/Base/MaxAlignT.hpp>

#include <cstddef>

static_assert(sizeof(sf::base::MaxAlignT) == sizeof(std::max_align_t));
static_assert(alignof(sf::base::MaxAlignT) == alignof(std::max_align_t));
