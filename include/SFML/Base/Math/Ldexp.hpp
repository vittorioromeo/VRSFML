#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(ldexp)
    #define SFML_BASE_MATH_LDEXP(...)  __builtin_ldexp(__VA_ARGS__)
    #define SFML_BASE_MATH_LDEXPF(...) __builtin_ldexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_LDEXPL(...) __builtin_ldexpl(__VA_ARGS__)
#else
    #define SFML_BASE_MATH_LDEXP(...)  ::std::ldexp(__VA_ARGS__)
    #define SFML_BASE_MATH_LDEXPF(...) ::std::ldexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_LDEXPL(...) ::std::ldexpl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
namespace sf::base
{

template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto ldexp(const T arg0, int exp) noexcept
{
    if constexpr (SFML_BASE_IS_SAME(T, float))
        return SFML_BASE_MATH_LDEXPF(arg0, exp);
    else if constexpr (SFML_BASE_IS_SAME(T, double))
        return SFML_BASE_MATH_LDEXP(arg0, exp);
    else if constexpr (SFML_BASE_IS_SAME(T, long double))
        return SFML_BASE_MATH_LDEXPL(arg0, exp);
    else
        static_assert(sizeof(T) == 0);
}

} // namespace sf::base


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
