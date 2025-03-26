#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(round)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"
    #
    #define SFML_BASE_MATH_ROUND(...)  __builtin_round(__VA_ARGS__)
    #define SFML_BASE_MATH_ROUNDF(...) __builtin_roundf(__VA_ARGS__)
    #define SFML_BASE_MATH_ROUNDL(...) __builtin_roundl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_ROUND(...)  ::std::round(__VA_ARGS__)
    #define SFML_BASE_MATH_ROUNDF(...) ::std::roundf(__VA_ARGS__)
    #define SFML_BASE_MATH_ROUNDL(...) ::std::roundl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(round)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
