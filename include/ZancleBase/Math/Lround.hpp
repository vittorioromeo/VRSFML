#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(lround)
    #define ZB_MATH_LROUND(...)  __builtin_lround(__VA_ARGS__)
    #define ZB_MATH_LROUNDF(...) __builtin_lroundf(__VA_ARGS__)
    #define ZB_MATH_LROUNDL(...) __builtin_lroundl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_LROUND(...)  ::std::lround(__VA_ARGS__)
    #define ZB_MATH_LROUNDF(...) ::std::lroundf(__VA_ARGS__)
    #define ZB_MATH_LROUNDL(...) ::std::lroundl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(lround, LROUND)
