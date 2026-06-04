#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(fabs)
    #define ZB_MATH_FABS(...)  __builtin_fabs(__VA_ARGS__)
    #define ZB_MATH_FABSF(...) __builtin_fabsf(__VA_ARGS__)
    #define ZB_MATH_FABSL(...) __builtin_fabsl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_FABS(...)  ::std::fabs(__VA_ARGS__)
    #define ZB_MATH_FABSF(...) ::std::fabsf(__VA_ARGS__)
    #define ZB_MATH_FABSL(...) ::std::fabsl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(fabs, FABS)
