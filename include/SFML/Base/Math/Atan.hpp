#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(atan)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_ATAN(...)  __builtin_atan(__VA_ARGS__)
    #define SFML_BASE_MATH_ATANF(...) __builtin_atanf(__VA_ARGS__)
    #define SFML_BASE_MATH_ATANL(...) __builtin_atanl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_ATAN(...)  ::std::atan(__VA_ARGS__)
    #define SFML_BASE_MATH_ATANF(...) ::std::atanf(__VA_ARGS__)
    #define SFML_BASE_MATH_ATANL(...) ::std::atanl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(atan)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
