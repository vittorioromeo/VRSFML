#include "ZancleBase/Constants.hpp"
#include "ZancleBase/SinCosLookup.hpp"


////////////////////////////////////////////////////////////
static_assert(sizeof(float) == 4);


////////////////////////////////////////////////////////////
static_assert(zb::priv::radToIndex >= 10430.f);
static_assert(zb::priv::radToIndex <= 10430.9f);


////////////////////////////////////////////////////////////
static_assert(zb::sinLookup(zb::halfPi * 0.f) == 0.f);
static_assert(zb::sinLookup(zb::halfPi * 0.5f) == 0.70710678118f);
static_assert(zb::sinLookup(zb::halfPi * 1.f) == 1.f);
static_assert(zb::sinLookup(zb::halfPi * 1.5f) == 0.70710678118f);
static_assert(zb::sinLookup(zb::halfPi * 2.f) == 0.f);
static_assert(zb::sinLookup(zb::halfPi * 2.5f) == -0.70710678118f);
static_assert(zb::sinLookup(zb::halfPi * 3.f) == -1.f);
static_assert(zb::sinLookup(zb::halfPi * 3.5f) == -0.70710678118f);
static_assert(zb::sinLookup(zb::halfPi * 4.f) == 0.f);


////////////////////////////////////////////////////////////
static_assert(zb::cosLookup(zb::halfPi * 0.f) == 1.f);
static_assert(zb::cosLookup(zb::halfPi * 0.5f) == 0.70710678118f);
static_assert(zb::cosLookup(zb::halfPi * 1.f) == 0.f);
static_assert(zb::cosLookup(zb::halfPi * 1.5f) == -0.70710678118f);
static_assert(zb::cosLookup(zb::halfPi * 2.f) == -1.f);
static_assert(zb::cosLookup(zb::halfPi * 2.5f) == -0.70710678118f);
static_assert(zb::cosLookup(zb::halfPi * 3.f) == 0.f);
static_assert(zb::cosLookup(zb::halfPi * 3.5f) == 0.70710678118f);
static_assert(zb::cosLookup(zb::halfPi * 4.f) == 1.f);
