#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(ceil)
    #define ZB_MATH_CEIL(...)  __builtin_ceil(__VA_ARGS__)
    #define ZB_MATH_CEILF(...) __builtin_ceilf(__VA_ARGS__)
    #define ZB_MATH_CEILL(...) __builtin_ceill(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_CEIL(...)  ::std::ceil(__VA_ARGS__)
    #define ZB_MATH_CEILF(...) ::std::ceilf(__VA_ARGS__)
    #define ZB_MATH_CEILL(...) ::std::ceill(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(ceil, CEIL)
