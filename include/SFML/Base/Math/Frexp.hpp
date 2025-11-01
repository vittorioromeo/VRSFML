#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(frexp)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_FREXP(...)  __builtin_frexp(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPF(...) __builtin_frexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPL(...) __builtin_frexpl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_FREXP(...)  ::std::frexp(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPF(...) ::std::frexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPL(...) ::std::frexpl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(frexp)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
