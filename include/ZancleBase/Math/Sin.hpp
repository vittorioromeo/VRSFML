#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(sin)
    #define ZB_MATH_SIN(...)  __builtin_sin(__VA_ARGS__)
    #define ZB_MATH_SINF(...) __builtin_sinf(__VA_ARGS__)
    #define ZB_MATH_SINL(...) __builtin_sinl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_SIN(...)  ::std::sin(__VA_ARGS__)
    #define ZB_MATH_SINF(...) ::std::sinf(__VA_ARGS__)
    #define ZB_MATH_SINL(...) ::std::sinl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(sin, SIN)
