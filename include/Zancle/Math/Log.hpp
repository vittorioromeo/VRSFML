#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "Zancle/Math/Priv/Impl.hpp"


////////////////////////////////////////////////////////////
#if ZA_PRIV_HAS_MATH_BUILTIN(log)
    #define ZA_MATH_LOG(...)  __builtin_log(__VA_ARGS__)
    #define ZA_MATH_LOGF(...) __builtin_logf(__VA_ARGS__)
    #define ZA_MATH_LOGL(...) __builtin_logl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZA_MATH_LOG(...)  ::std::log(__VA_ARGS__)
    #define ZA_MATH_LOGF(...) ::std::logf(__VA_ARGS__)
    #define ZA_MATH_LOGL(...) ::std::logl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
ZA_PRIV_DEFINE_MATH_WRAPPER_1ARG(log, LOG)
