#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Internal helpers for `sf::base::math::*` math wrappers
///
/// Provides `SFML_BASE_PRIV_HAS_MATH_BUILTIN(name)`, which checks
/// whether all three of `__builtin_<name>`, `__builtin_<name>f`, and
/// `__builtin_<name>l` are available. Used by per-function math
/// headers to decide whether to define `SFML_BASE_MATH_<NAME>(F|L)`
/// macros that point at the compiler builtins or at `::std::<name>`
/// from `<cmath>`.
///
/// Also provides `SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_1ARG(name, NAME)`
/// and its 2-arg counterpart, which generate the type-dispatching
/// `sf::base::<name>` function template that calls the appropriate
/// `SFML_BASE_MATH_<NAME>(F|L)` macro for `float`, `double`, or
/// `long double`. Caller must define those macros before invoking.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Trait/IsSame.hpp"


////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_EXPAND_CHECK_BUILTIN(a, b) __##a(b)

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_CHECK_BUILTIN(x) SFML_BASE_PRIV_EXPAND_CHECK_BUILTIN(has_builtin, x)

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_HAS_MATH_BUILTIN(name) \
    SFML_BASE_PRIV_CHECK_BUILTIN(name) && SFML_BASE_PRIV_CHECK_BUILTIN(name##f) && SFML_BASE_PRIV_CHECK_BUILTIN(name##l)


////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_1ARG(name, NAME)                                                      \
    namespace sf::base                                                                                           \
    {                                                                                                            \
                                                                                                                 \
    template <typename T>                                                                                        \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto name(const T arg) noexcept \
    {                                                                                                            \
        if constexpr (SFML_BASE_IS_SAME(T, float))                                                               \
            return SFML_BASE_MATH_##NAME##F(arg);                                                                \
        else if constexpr (SFML_BASE_IS_SAME(T, double))                                                         \
            return SFML_BASE_MATH_##NAME(arg);                                                                   \
        else if constexpr (SFML_BASE_IS_SAME(T, long double))                                                    \
            return SFML_BASE_MATH_##NAME##L(arg);                                                                \
        else                                                                                                     \
            static_assert(false);                                                                                \
    }                                                                                                            \
                                                                                                                 \
    } // namespace sf::base

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_MATH_WRAPPER_2ARG(name, NAME)                                                                     \
    namespace sf::base                                                                                                          \
    {                                                                                                                           \
                                                                                                                                \
    template <typename T>                                                                                                       \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto name(const T arg0, const T arg1) noexcept \
    {                                                                                                                           \
        if constexpr (SFML_BASE_IS_SAME(T, float))                                                                              \
            return SFML_BASE_MATH_##NAME##F(arg0, arg1);                                                                        \
        else if constexpr (SFML_BASE_IS_SAME(T, double))                                                                        \
            return SFML_BASE_MATH_##NAME(arg0, arg1);                                                                           \
        else if constexpr (SFML_BASE_IS_SAME(T, long double))                                                                   \
            return SFML_BASE_MATH_##NAME##L(arg0, arg1);                                                                        \
        else                                                                                                                    \
            static_assert(false);                                                                                               \
    }                                                                                                                           \
                                                                                                                                \
    } // namespace sf::base
