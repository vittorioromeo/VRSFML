#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(atan2)
    #include "SFML/Base/Math/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_ATAN2(...)  __builtin_atan2(__VA_ARGS__)
    #define SFML_BASE_MATH_ATAN2F(...) __builtin_atan2f(__VA_ARGS__)
    #define SFML_BASE_MATH_ATAN2L(...) __builtin_atan2l(__VA_ARGS__)
#else
    #include "SFML/Base/Math/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_ATAN2(...)  ::std::atan2(__VA_ARGS__)
    #define SFML_BASE_MATH_ATAN2F(...) ::std::atan2f(__VA_ARGS__)
    #define SFML_BASE_MATH_ATAN2L(...) ::std::atan2l(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(atan2)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/ImplUndef.hpp"
