#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(atan2)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_ATAN2(...)  __builtin_atan2(__VA_ARGS__)
    #define SFML_BASE_MATH_ATAN2F(...) __builtin_atan2f(__VA_ARGS__)
    #define SFML_BASE_MATH_ATAN2L(...) __builtin_atan2l(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_ATAN2(...)  ::std::atan2(__VA_ARGS__)
    #define SFML_BASE_MATH_ATAN2F(...) ::std::atan2f(__VA_ARGS__)
    #define SFML_BASE_MATH_ATAN2L(...) ::std::atan2l(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(atan2)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
