#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(log)
    #include "SFML/Base/Math/Priv/ImplBuiltinWrapper.hpp"

    #define SFML_BASE_MATH_LOG(...)  __builtin_log(__VA_ARGS__)
    #define SFML_BASE_MATH_LOGF(...) __builtin_logf(__VA_ARGS__)
    #define SFML_BASE_MATH_LOGL(...) __builtin_logl(__VA_ARGS__)
#else
    #include "SFML/Base/Math/Priv/ImplStdForwarder.hpp"

    #define SFML_BASE_MATH_LOG(...)  ::std::log(__VA_ARGS__)
    #define SFML_BASE_MATH_LOGF(...) ::std::logf(__VA_ARGS__)
    #define SFML_BASE_MATH_LOGL(...) ::std::logl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(log)


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
