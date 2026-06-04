#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(round)
    #define ZB_MATH_ROUND(...)  __builtin_round(__VA_ARGS__)
    #define ZB_MATH_ROUNDF(...) __builtin_roundf(__VA_ARGS__)
    #define ZB_MATH_ROUNDL(...) __builtin_roundl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_ROUND(...)  ::std::round(__VA_ARGS__)
    #define ZB_MATH_ROUNDF(...) ::std::roundf(__VA_ARGS__)
    #define ZB_MATH_ROUNDL(...) ::std::roundl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(round, ROUND)
