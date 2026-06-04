#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(fmod)
    #define ZB_MATH_FMOD(...)  __builtin_fmod(__VA_ARGS__)
    #define ZB_MATH_FMODF(...) __builtin_fmodf(__VA_ARGS__)
    #define ZB_MATH_FMODL(...) __builtin_fmodl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_FMOD(...)  ::std::fmod(__VA_ARGS__)
    #define ZB_MATH_FMODF(...) ::std::fmodf(__VA_ARGS__)
    #define ZB_MATH_FMODL(...) ::std::fmodl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZB_PRIV_DEFINE_MATH_WRAPPER_2ARG(fmod, FMOD)
