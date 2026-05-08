#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(fmin)
    #define SFML_BASE_MATH_FMIN(...)  __builtin_fmin(__VA_ARGS__)
    #define SFML_BASE_MATH_FMINF(...) __builtin_fminf(__VA_ARGS__)
    #define SFML_BASE_MATH_FMINL(...) __builtin_fminl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define SFML_BASE_MATH_FMIN(...)  ::std::fmin(__VA_ARGS__)
    #define SFML_BASE_MATH_FMINF(...) ::std::fminf(__VA_ARGS__)
    #define SFML_BASE_MATH_FMINL(...) ::std::fminl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_2ARG(fmin, FMIN)
