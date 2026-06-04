#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(log10)
    #define ZB_MATH_LOG10(...)  __builtin_log10(__VA_ARGS__)
    #define ZB_MATH_LOG10F(...) __builtin_log10f(__VA_ARGS__)
    #define ZB_MATH_LOG10L(...) __builtin_log10l(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_LOG10(...)  ::std::log10(__VA_ARGS__)
    #define ZB_MATH_LOG10F(...) ::std::log10f(__VA_ARGS__)
    #define ZB_MATH_LOG10L(...) ::std::log10l(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(log10, LOG10)
