#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(floor)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_FLOOR(...)  __builtin_floor(__VA_ARGS__)
    #define SFML_BASE_MATH_FLOORF(...) __builtin_floorf(__VA_ARGS__)
    #define SFML_BASE_MATH_FLOORL(...) __builtin_floorl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_FLOOR(...)  ::std::floor(__VA_ARGS__)
    #define SFML_BASE_MATH_FLOORF(...) ::std::floorf(__VA_ARGS__)
    #define SFML_BASE_MATH_FLOORL(...) ::std::floorl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(floor)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
