#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(fmod)
    #define SFML_BASE_MATH_FMOD(...)  __builtin_fmod(__VA_ARGS__)
    #define SFML_BASE_MATH_FMODF(...) __builtin_fmodf(__VA_ARGS__)
    #define SFML_BASE_MATH_FMODL(...) __builtin_fmodl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define SFML_BASE_MATH_FMOD(...)  ::std::fmod(__VA_ARGS__)
    #define SFML_BASE_MATH_FMODF(...) ::std::fmodf(__VA_ARGS__)
    #define SFML_BASE_MATH_FMODL(...) ::std::fmodl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_2ARG(fmod, FMOD)
