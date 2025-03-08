#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(fabs)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_FABS(...)  __builtin_fabs(__VA_ARGS__)
    #define SFML_BASE_MATH_FABSF(...) __builtin_fabsf(__VA_ARGS__)
    #define SFML_BASE_MATH_FABSL(...) __builtin_fabsl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_FABS(...)  ::std::fabs(__VA_ARGS__)
    #define SFML_BASE_MATH_FABSF(...) ::std::fabsf(__VA_ARGS__)
    #define SFML_BASE_MATH_FABSL(...) ::std::fabsl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(fabs)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
