#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(acos)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_ACOS(...)  __builtin_acos(__VA_ARGS__)
    #define SFML_BASE_MATH_ACOSF(...) __builtin_acosf(__VA_ARGS__)
    #define SFML_BASE_MATH_ACOSL(...) __builtin_acosl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_ACOS(...)  ::std::acos(__VA_ARGS__)
    #define SFML_BASE_MATH_ACOSF(...) ::std::acosf(__VA_ARGS__)
    #define SFML_BASE_MATH_ACOSL(...) ::std::acosl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(acos)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
