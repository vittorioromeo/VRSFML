#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(fmax)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_FMAX(...)  __builtin_fmax(__VA_ARGS__)
    #define SFML_BASE_MATH_FMAXF(...) __builtin_fmaxf(__VA_ARGS__)
    #define SFML_BASE_MATH_FMAXL(...) __builtin_fmaxl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_FMAX(...)  ::std::fmax(__VA_ARGS__)
    #define SFML_BASE_MATH_FMAXF(...) ::std::fmaxf(__VA_ARGS__)
    #define SFML_BASE_MATH_FMAXL(...) ::std::fmaxl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(fmax)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
