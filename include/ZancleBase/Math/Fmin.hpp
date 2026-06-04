#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(fmin)
    #define ZB_MATH_FMIN(...)  __builtin_fmin(__VA_ARGS__)
    #define ZB_MATH_FMINF(...) __builtin_fminf(__VA_ARGS__)
    #define ZB_MATH_FMINL(...) __builtin_fminl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_FMIN(...)  ::std::fmin(__VA_ARGS__)
    #define ZB_MATH_FMINF(...) ::std::fminf(__VA_ARGS__)
    #define ZB_MATH_FMINL(...) ::std::fminl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_2ARG(fmin, FMIN)
