#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(floor)
    #define ZB_MATH_FLOOR(...)  __builtin_floor(__VA_ARGS__)
    #define ZB_MATH_FLOORF(...) __builtin_floorf(__VA_ARGS__)
    #define ZB_MATH_FLOORL(...) __builtin_floorl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_FLOOR(...)  ::std::floor(__VA_ARGS__)
    #define ZB_MATH_FLOORF(...) ::std::floorf(__VA_ARGS__)
    #define ZB_MATH_FLOORL(...) ::std::floorl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(floor, FLOOR)
