#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(asin)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_ASIN(...)  __builtin_asin(__VA_ARGS__)
    #define SFML_BASE_MATH_ASINF(...) __builtin_asinf(__VA_ARGS__)
    #define SFML_BASE_MATH_ASINL(...) __builtin_asinl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_ASIN(...)  ::std::asin(__VA_ARGS__)
    #define SFML_BASE_MATH_ASINF(...) ::std::asinf(__VA_ARGS__)
    #define SFML_BASE_MATH_ASINL(...) ::std::asinl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(asin)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
