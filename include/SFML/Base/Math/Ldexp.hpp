#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(ldexp)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_LDEXP(...)  __builtin_ldexp(__VA_ARGS__)
    #define SFML_BASE_MATH_LDEXPF(...) __builtin_ldexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_LDEXPL(...) __builtin_ldexpl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_LDEXP(...)  ::std::ldexp(__VA_ARGS__)
    #define SFML_BASE_MATH_LDEXPF(...) ::std::ldexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_LDEXPL(...) ::std::ldexpl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(ldexp)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
