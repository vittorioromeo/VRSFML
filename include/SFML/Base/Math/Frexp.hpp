#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/Impl.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


////////////////////////////////////////////////////////////
#if SFML_BASE_PRIV_HAS_MATH_BUILTIN(frexp)
    #define SFML_BASE_MATH_FREXP(...)  __builtin_frexp(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPF(...) __builtin_frexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPL(...) __builtin_frexpl(__VA_ARGS__)
#else
    #define SFML_BASE_MATH_FREXP(...)  ::std::frexp(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPF(...) ::std::frexpf(__VA_ARGS__)
    #define SFML_BASE_MATH_FREXPL(...) ::std::frexpl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
namespace sf::base
{

template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto frexp(const T arg0, int* exp) noexcept
{
    if constexpr (SFML_BASE_IS_SAME(T, float))
        return SFML_BASE_MATH_FREXPF(arg0, exp);
    else if constexpr (SFML_BASE_IS_SAME(T, double))
        return SFML_BASE_MATH_FREXP(arg0, exp);
    else if constexpr (SFML_BASE_IS_SAME(T, long double))
        return SFML_BASE_MATH_FREXPL(arg0, exp);
    else
        static_assert(sizeof(T) == 0);
}

} // namespace sf::base


////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Priv/ImplUndef.hpp"
