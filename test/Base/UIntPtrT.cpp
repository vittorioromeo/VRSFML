#include "SFML/Base/UIntPtrT.hpp"

#include <cstdint>


////////////////////////////////////////////////////////////
static_assert(sizeof(sf::base::UIntPtrT) == sizeof(std::uintptr_t));
static_assert(alignof(sf::base::UIntPtrT) == alignof(std::uintptr_t));
