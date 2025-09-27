#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(pow)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_POW(...)  __builtin_pow(__VA_ARGS__)
    #define SFML_BASE_MATH_POWF(...) __builtin_powf(__VA_ARGS__)
    #define SFML_BASE_MATH_POWL(...) __builtin_powl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_POW(...)  ::std::pow(__VA_ARGS__)
    #define SFML_BASE_MATH_POWF(...) ::std::powf(__VA_ARGS__)
    #define SFML_BASE_MATH_POWL(...) ::std::powl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(pow)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
