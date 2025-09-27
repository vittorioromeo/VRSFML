#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(sin)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_SIN(...)  __builtin_sin(__VA_ARGS__)
    #define SFML_BASE_MATH_SINF(...) __builtin_sinf(__VA_ARGS__)
    #define SFML_BASE_MATH_SINL(...) __builtin_sinl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_SIN(...)  ::std::sin(__VA_ARGS__)
    #define SFML_BASE_MATH_SINF(...) ::std::sinf(__VA_ARGS__)
    #define SFML_BASE_MATH_SINL(...) ::std::sinl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(sin)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
