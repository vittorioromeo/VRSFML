#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "Zancle/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZA_PRIV_HAS_MATH_BUILTIN(floor)
    #define ZA_MATH_FLOOR(...)  __builtin_floor(__VA_ARGS__)
    #define ZA_MATH_FLOORF(...) __builtin_floorf(__VA_ARGS__)
    #define ZA_MATH_FLOORL(...) __builtin_floorl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZA_MATH_FLOOR(...)  ::std::floor(__VA_ARGS__)
    #define ZA_MATH_FLOORF(...) ::std::floorf(__VA_ARGS__)
    #define ZA_MATH_FLOORL(...) ::std::floorl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZA_PRIV_DEFINE_MATH_WRAPPER_1ARG(floor, FLOOR)
