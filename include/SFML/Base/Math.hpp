#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/Traits/IsSame.hpp>


#if __has_builtin(__builtin_fabs) && __has_builtin(__builtin_fabsf) && __has_builtin(__builtin_fabsl) &&       \
    __has_builtin(__builtin_atan2) && __has_builtin(__builtin_atan2f) && __has_builtin(__builtin_atan2l) &&    \
    __has_builtin(__builtin_ceil) && __has_builtin(__builtin_ceilf) && __has_builtin(__builtin_ceill) &&       \
    __has_builtin(__builtin_cos) && __has_builtin(__builtin_cosf) && __has_builtin(__builtin_cosl) &&          \
    __has_builtin(__builtin_floor) && __has_builtin(__builtin_floorf) && __has_builtin(__builtin_floorl) &&    \
    __has_builtin(__builtin_lround) && __has_builtin(__builtin_lroundf) && __has_builtin(__builtin_lroundl) && \
    __has_builtin(__builtin_sin) && __has_builtin(__builtin_sinf) && __has_builtin(__builtin_sinl) &&          \
    __has_builtin(__builtin_sqrt) && __has_builtin(__builtin_sqrtf) && __has_builtin(__builtin_sqrtl)

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_HAS_MATH_BUILTINS 1

#endif

#ifndef SFML_BASE_PRIV_HAS_MATH_BUILTINS
#include <cmath>
#endif

#ifdef SFML_BASE_PRIV_HAS_MATH_BUILTINS

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(name)                                \
    template <typename T>                                                                    \
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto name(T arg) noexcept \
    {                                                                                        \
        if constexpr (SFML_BASE_IS_SAME(T, float))                                           \
        {                                                                                    \
            return __builtin_##name##f(arg);                                                 \
        }                                                                                    \
        else if constexpr (SFML_BASE_IS_SAME(T, double))                                     \
        {                                                                                    \
            return __builtin_##name(arg);                                                    \
        }                                                                                    \
        else if constexpr (SFML_BASE_IS_SAME(T, long double))                                \
        {                                                                                    \
            return __builtin_##name##l(arg);                                                 \
        }                                                                                    \
    }

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(name)                                         \
    template <typename T>                                                                             \
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto name(T arg0, T arg1) noexcept \
    {                                                                                                 \
        if constexpr (SFML_BASE_IS_SAME(T, float))                                                    \
        {                                                                                             \
            return __builtin_##name##f(arg0, arg1);                                                   \
        }                                                                                             \
        else if constexpr (SFML_BASE_IS_SAME(T, double))                                              \
        {                                                                                             \
            return __builtin_##name(arg0, arg1);                                                      \
        }                                                                                             \
        else if constexpr (SFML_BASE_IS_SAME(T, long double))                                         \
        {                                                                                             \
            return __builtin_##name##l(arg0, arg1);                                                   \
        }                                                                                             \
    }

#else

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(name)                                \
    template <typename T>                                                                    \
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto name(T arg) noexcept \
    {                                                                                        \
        return std::name(arg);                                                               \
    }

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(name)                                         \
    template <typename T>                                                                             \
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto name(T arg0, T arg1) noexcept \
    {                                                                                                 \
        return std::name(arg0, arg1);                                                                 \
    }

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(fabs)

////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(atan2)

////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(ceil)

////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(cos)

////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(floor)

////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(lround)

////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(sin)

////////////////////////////////////////////////////////////
SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(sqrt)

} // namespace sf::base
