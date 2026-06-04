#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(asin)
    #define ZB_MATH_ASIN(...)  __builtin_asin(__VA_ARGS__)
    #define ZB_MATH_ASINF(...) __builtin_asinf(__VA_ARGS__)
    #define ZB_MATH_ASINL(...) __builtin_asinl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_ASIN(...)  ::std::asin(__VA_ARGS__)
    #define ZB_MATH_ASINF(...) ::std::asinf(__VA_ARGS__)
    #define ZB_MATH_ASINL(...) ::std::asinl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(asin, ASIN)
