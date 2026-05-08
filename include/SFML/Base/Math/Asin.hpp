#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(asin)
    #define SFML_BASE_MATH_ASIN(...)  __builtin_asin(__VA_ARGS__)
    #define SFML_BASE_MATH_ASINF(...) __builtin_asinf(__VA_ARGS__)
    #define SFML_BASE_MATH_ASINL(...) __builtin_asinl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define SFML_BASE_MATH_ASIN(...)  ::std::asin(__VA_ARGS__)
    #define SFML_BASE_MATH_ASINF(...) ::std::asinf(__VA_ARGS__)
    #define SFML_BASE_MATH_ASINL(...) ::std::asinl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_1ARG(asin, ASIN)
