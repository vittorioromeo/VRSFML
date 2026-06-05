#include "Zancle/Math/SinCosLookup.hpp"

#include "Zancle/Math/Constants.hpp"


////////////////////////////////////////////////////////////
static_assert(sizeof(float) == 4);


////////////////////////////////////////////////////////////
static_assert(za::priv::radToIndex >= 10430.f);
static_assert(za::priv::radToIndex <= 10430.9f);


////////////////////////////////////////////////////////////
static_assert(za::sinLookup(za::halfPi * 0.f) == 0.f);
static_assert(za::sinLookup(za::halfPi * 0.5f) == 0.70710678118f);
static_assert(za::sinLookup(za::halfPi * 1.f) == 1.f);
static_assert(za::sinLookup(za::halfPi * 1.5f) == 0.70710678118f);
static_assert(za::sinLookup(za::halfPi * 2.f) == 0.f);
static_assert(za::sinLookup(za::halfPi * 2.5f) == -0.70710678118f);
static_assert(za::sinLookup(za::halfPi * 3.f) == -1.f);
static_assert(za::sinLookup(za::halfPi * 3.5f) == -0.70710678118f);
static_assert(za::sinLookup(za::halfPi * 4.f) == 0.f);


////////////////////////////////////////////////////////////
static_assert(za::cosLookup(za::halfPi * 0.f) == 1.f);
static_assert(za::cosLookup(za::halfPi * 0.5f) == 0.70710678118f);
static_assert(za::cosLookup(za::halfPi * 1.f) == 0.f);
static_assert(za::cosLookup(za::halfPi * 1.5f) == -0.70710678118f);
static_assert(za::cosLookup(za::halfPi * 2.f) == -1.f);
static_assert(za::cosLookup(za::halfPi * 2.5f) == -0.70710678118f);
static_assert(za::cosLookup(za::halfPi * 3.f) == 0.f);
static_assert(za::cosLookup(za::halfPi * 3.5f) == 0.70710678118f);
static_assert(za::cosLookup(za::halfPi * 4.f) == 1.f);
