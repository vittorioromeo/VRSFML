#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(lround)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_LROUND(...)  __builtin_lround(__VA_ARGS__)
    #define SFML_BASE_MATH_LROUNDF(...) __builtin_lroundf(__VA_ARGS__)
    #define SFML_BASE_MATH_LROUNDL(...) __builtin_lroundl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_LROUND(...)  ::std::lround(__VA_ARGS__)
    #define SFML_BASE_MATH_LROUNDF(...) ::std::lroundf(__VA_ARGS__)
    #define SFML_BASE_MATH_LROUNDL(...) ::std::lroundl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(lround)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
