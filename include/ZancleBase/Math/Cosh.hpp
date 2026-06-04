#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(cosh)
    #define ZB_MATH_COSH(...)  __builtin_cosh(__VA_ARGS__)
    #define ZB_MATH_COSHF(...) __builtin_coshf(__VA_ARGS__)
    #define ZB_MATH_COSHL(...) __builtin_coshl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_COSH(...)  ::std::cosh(__VA_ARGS__)
    #define ZB_MATH_COSHF(...) ::std::coshf(__VA_ARGS__)
    #define ZB_MATH_COSHL(...) ::std::coshl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(cosh, COSH)
