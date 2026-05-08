#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(exp)
    #define SFML_BASE_MATH_EXP(...)  __builtin_exp(__VA_ARGS__)
    #define SFML_BASE_MATH_EXPF(...) __builtin_expf(__VA_ARGS__)
    #define SFML_BASE_MATH_EXPL(...) __builtin_expl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define SFML_BASE_MATH_EXP(...)  ::std::exp(__VA_ARGS__)
    #define SFML_BASE_MATH_EXPF(...) ::std::expf(__VA_ARGS__)
    #define SFML_BASE_MATH_EXPL(...) ::std::expl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_1ARG(exp, EXP)
