#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Internal helpers for `zb::math::*` math wrappers
///
/// Provides `ZB_PRIV_HAS_MATH_BUILTIN(name)`, which checks
/// whether all three of `__builtin_<name>`, `__builtin_<name>f`, and
/// `__builtin_<name>l` are available. Used by per-function math
/// headers to decide whether to define `ZB_MATH_<NAME>(F|L)`
/// macros that point at the compiler builtins or at `::std::<name>`
/// from `<cmath>`.
///
/// Also provides `ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(name, NAME)`
/// and its 2-arg counterpart, which generate the type-dispatching
/// `zb::<name>` function template that calls the appropriate
/// `ZB_MATH_<NAME>(F|L)` macro for `float`, `double`, or
/// `long double`. Caller must define those macros before invoking.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Trait/IsSame.hpp"


////////////////////////////////////////////////////////////
#define ZB_PRIV_EXPAND_CHECK_BUILTIN(a, b) __##a(b)

////////////////////////////////////////////////////////////
#define ZB_PRIV_CHECK_BUILTIN(x) ZB_PRIV_EXPAND_CHECK_BUILTIN(has_builtin, x)

////////////////////////////////////////////////////////////
#define ZB_PRIV_HAS_MATH_BUILTIN(name) \
    ZB_PRIV_CHECK_BUILTIN(name) && ZB_PRIV_CHECK_BUILTIN(name##f) && ZB_PRIV_CHECK_BUILTIN(name##l)


////////////////////////////////////////////////////////////
#define ZB_PRIV_DEFINE_MATH_WRAPPER_1ARG(name, NAME)                                                             \
    namespace zb                                                                                                 \
    {                                                                                                            \
                                                                                                                 \
    template <typename T>                                                                                        \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto name(const T arg) noexcept \
    {                                                                                                            \
        if constexpr (ZB_IS_SAME(T, float))                                                                      \
            return ZB_MATH_##NAME##F(arg);                                                                       \
        else if constexpr (ZB_IS_SAME(T, double))                                                                \
            return ZB_MATH_##NAME(arg);                                                                          \
        else if constexpr (ZB_IS_SAME(T, long double))                                                           \
            return ZB_MATH_##NAME##L(arg);                                                                       \
        else                                                                                                     \
            static_assert(false);                                                                                \
    }                                                                                                            \
                                                                                                                 \
    } // namespace zb

////////////////////////////////////////////////////////////
#define ZB_PRIV_DEFINE_MATH_WRAPPER_2ARG(name, NAME)                                                                            \
    namespace zb                                                                                                                \
    {                                                                                                                           \
                                                                                                                                \
    template <typename T>                                                                                                       \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto name(const T arg0, const T arg1) noexcept \
    {                                                                                                                           \
        if constexpr (ZB_IS_SAME(T, float))                                                                                     \
            return ZB_MATH_##NAME##F(arg0, arg1);                                                                               \
        else if constexpr (ZB_IS_SAME(T, double))                                                                               \
            return ZB_MATH_##NAME(arg0, arg1);                                                                                  \
        else if constexpr (ZB_IS_SAME(T, long double))                                                                          \
            return ZB_MATH_##NAME##L(arg0, arg1);                                                                               \
        else                                                                                                                    \
            static_assert(false);                                                                                               \
    }                                                                                                                           \
                                                                                                                                \
    } // namespace zb
