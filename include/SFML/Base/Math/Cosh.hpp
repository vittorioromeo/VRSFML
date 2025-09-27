#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(cosh)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_COSH(...)  __builtin_cosh(__VA_ARGS__)
    #define SFML_BASE_MATH_COSHF(...) __builtin_coshf(__VA_ARGS__)
    #define SFML_BASE_MATH_COSHL(...) __builtin_coshl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_COSH(...)  ::std::cosh(__VA_ARGS__)
    #define SFML_BASE_MATH_COSHF(...) ::std::coshf(__VA_ARGS__)
    #define SFML_BASE_MATH_COSHL(...) ::std::coshl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(cosh)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
