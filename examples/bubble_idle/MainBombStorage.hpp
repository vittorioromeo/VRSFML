#pragma once

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct MainBombStorage
{
    ankerl::unordered_dense::map<sf::base::SizeT, sf::base::SizeT> bombIdxToCatIdx;
};
