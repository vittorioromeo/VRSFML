#include <SFML/Base/SizeT.hpp>

#include <cstddef>

static_assert(sizeof(sf::base::SizeT) == sizeof(std::size_t));
static_assert(alignof(sf::base::SizeT) == alignof(std::size_t));
