#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(frexp)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_FREXP(...)  __builtin_frexp(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPF(...) __builtin_frexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPL(...) __builtin_frexpl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_FREXP(...)  ::std::frexp(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPF(...) ::std::frexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPL(...) ::std::frexpl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(frexp)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
