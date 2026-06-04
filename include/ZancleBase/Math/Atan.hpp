#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(atan)
    #define ZB_MATH_ATAN(...)  __builtin_atan(__VA_ARGS__)
    #define ZB_MATH_ATANF(...) __builtin_atanf(__VA_ARGS__)
    #define ZB_MATH_ATANL(...) __builtin_atanl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_ATAN(...)  ::std::atan(__VA_ARGS__)
    #define ZB_MATH_ATANF(...) ::std::atanf(__VA_ARGS__)
    #define ZB_MATH_ATANL(...) ::std::atanl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(atan, ATAN)
