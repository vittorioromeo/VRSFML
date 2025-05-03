#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(tan)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_TAN(...)  __builtin_tan(__VA_ARGS__)
    #define SFML_BASE_MATH_TANF(...) __builtin_tanf(__VA_ARGS__)
    #define SFML_BASE_MATH_TANL(...) __builtin_tanl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_TAN(...)  ::std::tan(__VA_ARGS__)
    #define SFML_BASE_MATH_TANF(...) ::std::tanf(__VA_ARGS__)
    #define SFML_BASE_MATH_TANL(...) ::std::tanl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(tan)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
