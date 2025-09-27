#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(fmod)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_FMOD(...)  __builtin_fmod(__VA_ARGS__)
    #define SFML_BASE_MATH_FMODF(...) __builtin_fmodf(__VA_ARGS__)
    #define SFML_BASE_MATH_FMODL(...) __builtin_fmodl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_FMOD(...)  ::std::fmod(__VA_ARGS__)
    #define SFML_BASE_MATH_FMODF(...) ::std::fmodf(__VA_ARGS__)
    #define SFML_BASE_MATH_FMODL(...) ::std::fmodl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(fmod)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
