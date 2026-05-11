#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(rint)
    #define SFML_BASE_MATH_RINT(...)  __builtin_rint(__VA_ARGS__)
    #define SFML_BASE_MATH_RINTF(...) __builtin_rintf(__VA_ARGS__)
    #define SFML_BASE_MATH_RINTL(...) __builtin_rintl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define SFML_BASE_MATH_RINT(...)  ::std::rint(__VA_ARGS__)
    #define SFML_BASE_MATH_RINTF(...) ::std::rintf(__VA_ARGS__)
    #define SFML_BASE_MATH_RINTL(...) ::std::rintl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_1ARG(rint, RINT)
