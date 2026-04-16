#include "SFML/Base/PtrDiffT.hpp"

#include <cstddef>


////////////////////////////////////////////////////////////
static_assert(sizeof(sf::base::PtrDiffT) == sizeof(std::ptrdiff_t));
static_assert(alignof(sf::base::PtrDiffT) == alignof(std::ptrdiff_t));
