#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(acos)
    #define ZB_MATH_ACOS(...)  __builtin_acos(__VA_ARGS__)
    #define ZB_MATH_ACOSF(...) __builtin_acosf(__VA_ARGS__)
    #define ZB_MATH_ACOSL(...) __builtin_acosl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_ACOS(...)  ::std::acos(__VA_ARGS__)
    #define ZB_MATH_ACOSF(...) ::std::acosf(__VA_ARGS__)
    #define ZB_MATH_ACOSL(...) ::std::acosl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(acos, ACOS)
