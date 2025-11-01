#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(cos)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_COS(...)  __builtin_cos(__VA_ARGS__)
    #define SFML_BASE_MATH_COSF(...) __builtin_cosf(__VA_ARGS__)
    #define SFML_BASE_MATH_COSL(...) __builtin_cosl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_COS(...)  ::std::cos(__VA_ARGS__)
    #define SFML_BASE_MATH_COSF(...) ::std::cosf(__VA_ARGS__)
    #define SFML_BASE_MATH_COSL(...) ::std::cosl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(cos)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
