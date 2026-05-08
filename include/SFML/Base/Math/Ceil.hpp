#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(ceil)
    #define SFML_BASE_MATH_CEIL(...)  __builtin_ceil(__VA_ARGS__)
    #define SFML_BASE_MATH_CEILF(...) __builtin_ceilf(__VA_ARGS__)
    #define SFML_BASE_MATH_CEILL(...) __builtin_ceill(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define SFML_BASE_MATH_CEIL(...)  ::std::ceil(__VA_ARGS__)
    #define SFML_BASE_MATH_CEILF(...) ::std::ceilf(__VA_ARGS__)
    #define SFML_BASE_MATH_CEILL(...) ::std::ceill(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_1ARG(ceil, CEIL)
