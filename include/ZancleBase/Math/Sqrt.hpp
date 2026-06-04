#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(sqrt)
    #define ZB_MATH_SQRT(...)  __builtin_sqrt(__VA_ARGS__)
    #define ZB_MATH_SQRTF(...) __builtin_sqrtf(__VA_ARGS__)
    #define ZB_MATH_SQRTL(...) __builtin_sqrtl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_SQRT(...)  ::std::sqrt(__VA_ARGS__)
    #define ZB_MATH_SQRTF(...) ::std::sqrtf(__VA_ARGS__)
    #define ZB_MATH_SQRTL(...) ::std::sqrtl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(sqrt, SQRT)
