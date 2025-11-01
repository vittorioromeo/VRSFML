#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(round)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"
    #
    #define SFML_BASE_MATH_ROUND(...)  __builtin_round(__VA_ARGS__)
    #define SFML_BASE_MATH_ROUNDF(...) __builtin_roundf(__VA_ARGS__)
    #define SFML_BASE_MATH_ROUNDL(...) __builtin_roundl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_ROUND(...)  ::std::round(__VA_ARGS__)
    #define SFML_BASE_MATH_ROUNDF(...) ::std::roundf(__VA_ARGS__)
    #define SFML_BASE_MATH_ROUNDL(...) ::std::roundl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(round)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
