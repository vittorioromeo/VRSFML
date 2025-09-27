#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(ceil)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_CEIL(...)  __builtin_ceil(__VA_ARGS__)
    #define SFML_BASE_MATH_CEILF(...) __builtin_ceilf(__VA_ARGS__)
    #define SFML_BASE_MATH_CEILL(...) __builtin_ceill(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_CEIL(...)  ::std::ceil(__VA_ARGS__)
    #define SFML_BASE_MATH_CEILF(...) ::std::ceilf(__VA_ARGS__)
    #define SFML_BASE_MATH_CEILL(...) ::std::ceill(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(ceil)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
