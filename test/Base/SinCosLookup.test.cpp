#include "SFML/Base/SinCosLookup.hpp"

#include "SFML/Base/Constants.hpp"


////////////////////////////////////////////////////////////
static_assert(sizeof(float) == 4);


////////////////////////////////////////////////////////////
static_assert(sf::base::priv::radToIndex >= 10430.f);
static_assert(sf::base::priv::radToIndex <= 10430.9f);


////////////////////////////////////////////////////////////
static_assert(sf::base::sinLookup(sf::base::halfPi * 0.f) == 0.f);
static_assert(sf::base::sinLookup(sf::base::halfPi * 0.5f) == 0.70710678118f);
static_assert(sf::base::sinLookup(sf::base::halfPi * 1.f) == 1.f);
static_assert(sf::base::sinLookup(sf::base::halfPi * 1.5f) == 0.70710678118f);
static_assert(sf::base::sinLookup(sf::base::halfPi * 2.f) == 0.f);
static_assert(sf::base::sinLookup(sf::base::halfPi * 2.5f) == -0.70710678118f);
static_assert(sf::base::sinLookup(sf::base::halfPi * 3.f) == -1.f);
static_assert(sf::base::sinLookup(sf::base::halfPi * 3.5f) == -0.70710678118f);
static_assert(sf::base::sinLookup(sf::base::halfPi * 4.f) == 0.f);


////////////////////////////////////////////////////////////
static_assert(sf::base::cosLookup(sf::base::halfPi * 0.f) == 1.f);
static_assert(sf::base::cosLookup(sf::base::halfPi * 0.5f) == 0.70710678118f);
static_assert(sf::base::cosLookup(sf::base::halfPi * 1.f) == 0.f);
static_assert(sf::base::cosLookup(sf::base::halfPi * 1.5f) == -0.70710678118f);
static_assert(sf::base::cosLookup(sf::base::halfPi * 2.f) == -1.f);
static_assert(sf::base::cosLookup(sf::base::halfPi * 2.5f) == -0.70710678118f);
static_assert(sf::base::cosLookup(sf::base::halfPi * 3.f) == 0.f);
static_assert(sf::base::cosLookup(sf::base::halfPi * 3.5f) == 0.70710678118f);
static_assert(sf::base::cosLookup(sf::base::halfPi * 4.f) == 1.f);
