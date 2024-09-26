#include "SFML/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
static_assert(sizeof(sf::base::I8) == 1);
static_assert(sizeof(sf::base::U8) == 1);


////////////////////////////////////////////////////////////
static_assert(sizeof(sf::base::I16) == 2);
static_assert(sizeof(sf::base::U16) == 2);


////////////////////////////////////////////////////////////
static_assert(sizeof(sf::base::I32) == 4);
static_assert(sizeof(sf::base::U32) == 4);


////////////////////////////////////////////////////////////
static_assert(sizeof(sf::base::I64) == 8);
static_assert(sizeof(sf::base::U64) == 8);
