#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "Zancle/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZA_PRIV_HAS_MATH_BUILTIN(tan)
    #define ZA_MATH_TAN(...)  __builtin_tan(__VA_ARGS__)
    #define ZA_MATH_TANF(...) __builtin_tanf(__VA_ARGS__)
    #define ZA_MATH_TANL(...) __builtin_tanl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZA_MATH_TAN(...)  ::std::tan(__VA_ARGS__)
    #define ZA_MATH_TANF(...) ::std::tanf(__VA_ARGS__)
    #define ZA_MATH_TANL(...) ::std::tanl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZA_PRIV_DEFINE_MATH_WRAPPER_1ARG(tan, TAN)
