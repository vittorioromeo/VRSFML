#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(pow)
    #define ZB_MATH_POW(...)  __builtin_pow(__VA_ARGS__)
    #define ZB_MATH_POWF(...) __builtin_powf(__VA_ARGS__)
    #define ZB_MATH_POWL(...) __builtin_powl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_POW(...)  ::std::pow(__VA_ARGS__)
    #define ZB_MATH_POWF(...) ::std::powf(__VA_ARGS__)
    #define ZB_MATH_POWL(...) ::std::powl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_2ARG(pow, POW)
