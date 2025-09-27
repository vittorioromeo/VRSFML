#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(exp)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_EXP(...)  __builtin_exp(__VA_ARGS__)
    #define SFML_BASE_MATH_EXPF(...) __builtin_expf(__VA_ARGS__)
    #define SFML_BASE_MATH_EXPL(...) __builtin_expl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_EXP(...)  ::std::exp(__VA_ARGS__)
    #define SFML_BASE_MATH_EXPF(...) ::std::expf(__VA_ARGS__)
    #define SFML_BASE_MATH_EXPL(...) ::std::expl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(exp)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
