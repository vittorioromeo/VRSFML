#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(log10)
    #define SFML_BASE_MATH_LOG10(...)  __builtin_log10(__VA_ARGS__)
    #define SFML_BASE_MATH_LOG10F(...) __builtin_log10f(__VA_ARGS__)
    #define SFML_BASE_MATH_LOG10L(...) __builtin_log10l(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define SFML_BASE_MATH_LOG10(...)  ::std::log10(__VA_ARGS__)
    #define SFML_BASE_MATH_LOG10F(...) ::std::log10f(__VA_ARGS__)
    #define SFML_BASE_MATH_LOG10L(...) ::std::log10l(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_1ARG(log10, LOG10)
