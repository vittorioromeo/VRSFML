#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(sqrt)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_SQRT(...)  __builtin_sqrt(__VA_ARGS__)
    #define SFML_BASE_MATH_SQRTF(...) __builtin_sqrtf(__VA_ARGS__)
    #define SFML_BASE_MATH_SQRTL(...) __builtin_sqrtl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_SQRT(...)  ::std::sqrt(__VA_ARGS__)
    #define SFML_BASE_MATH_SQRTF(...) ::std::sqrtf(__VA_ARGS__)
    #define SFML_BASE_MATH_SQRTL(...) ::std::sqrtl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(sqrt)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
