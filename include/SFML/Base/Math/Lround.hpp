#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(lround)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_LROUND(...)  __builtin_lround(__VA_ARGS__)
    #define SFML_BASE_MATH_LROUNDF(...) __builtin_lroundf(__VA_ARGS__)
    #define SFML_BASE_MATH_LROUNDL(...) __builtin_lroundl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_LROUND(...)  ::std::lround(__VA_ARGS__)
    #define SFML_BASE_MATH_LROUNDF(...) ::std::lroundf(__VA_ARGS__)
    #define SFML_BASE_MATH_LROUNDL(...) ::std::lroundl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(lround)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
