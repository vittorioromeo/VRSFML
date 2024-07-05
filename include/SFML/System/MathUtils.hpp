////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/IsSame.hpp>


#if __has_builtin(__builtin_fabs) && __has_builtin(__builtin_fabsf) && __has_builtin(__builtin_fabsl) &&       \
    __has_builtin(__builtin_atan2) && __has_builtin(__builtin_atan2f) && __has_builtin(__builtin_atan2l) &&    \
    __has_builtin(__builtin_ceil) && __has_builtin(__builtin_ceilf) && __has_builtin(__builtin_ceill) &&       \
    __has_builtin(__builtin_cos) && __has_builtin(__builtin_cosf) && __has_builtin(__builtin_cosl) &&          \
    __has_builtin(__builtin_floor) && __has_builtin(__builtin_floorf) && __has_builtin(__builtin_floorl) &&    \
    __has_builtin(__builtin_lround) && __has_builtin(__builtin_lroundf) && __has_builtin(__builtin_lroundl) && \
    __has_builtin(__builtin_sin) && __has_builtin(__builtin_sinf) && __has_builtin(__builtin_sinl) &&          \
    __has_builtin(__builtin_sqrt) && __has_builtin(__builtin_sqrtf) && __has_builtin(__builtin_sqrtl)

#define SFML_PRIV_HAS_MATH_BUILTINS 1

#endif

#ifndef SFML_PRIV_HAS_MATH_BUILTINS
#include <cmath>
#endif

#ifdef SFML_PRIV_HAS_MATH_BUILTINS

#define SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(name)                         \
    template <typename T>                                                        \
    [[nodiscard, gnu::always_inline]] inline constexpr auto name(T arg) noexcept \
    {                                                                            \
        if constexpr (SFML_PRIV_IS_SAME(T, float))                               \
        {                                                                        \
            return __builtin_##name##f(arg);                                     \
        }                                                                        \
        else if constexpr (SFML_PRIV_IS_SAME(T, double))                         \
        {                                                                        \
            return __builtin_##name(arg);                                        \
        }                                                                        \
        else if constexpr (SFML_PRIV_IS_SAME(T, long double))                    \
        {                                                                        \
            return __builtin_##name##l(arg);                                     \
        }                                                                        \
    }

#define SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(name)                                  \
    template <typename T>                                                                 \
    [[nodiscard, gnu::always_inline]] inline constexpr auto name(T arg0, T arg1) noexcept \
    {                                                                                     \
        if constexpr (SFML_PRIV_IS_SAME(T, float))                                        \
        {                                                                                 \
            return __builtin_##name##f(arg0, arg1);                                       \
        }                                                                                 \
        else if constexpr (SFML_PRIV_IS_SAME(T, double))                                  \
        {                                                                                 \
            return __builtin_##name(arg0, arg1);                                          \
        }                                                                                 \
        else if constexpr (SFML_PRIV_IS_SAME(T, long double))                             \
        {                                                                                 \
            return __builtin_##name##l(arg0, arg1);                                       \
        }                                                                                 \
    }

#else

#define SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(name)                         \
    template <typename T>                                                        \
    [[nodiscard, gnu::always_inline]] inline constexpr auto name(T arg) noexcept \
    {                                                                            \
        return std::name(arg);                                                   \
    }

#define SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(name)                                  \
    template <typename T>                                                                 \
    [[nodiscard, gnu::always_inline]] inline constexpr auto name(T arg0, T arg1) noexcept \
    {                                                                                     \
        return std::name(arg0, arg1);                                                     \
    }

#endif


namespace sf::priv
{

SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(fabs)
SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(atan2)
SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(ceil)
SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(cos)
SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(floor)
SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(lround)
SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(sin)
SFML_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(sqrt)

} // namespace sf::priv
