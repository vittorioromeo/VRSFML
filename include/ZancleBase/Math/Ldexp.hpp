#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"
#include "ZancleBase/Trait/IsSame.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(ldexp)
    #define ZB_MATH_LDEXP(...)  __builtin_ldexp(__VA_ARGS__)
    #define ZB_MATH_LDEXPF(...) __builtin_ldexpf(__VA_ARGS__)
    #define ZB_MATH_LDEXPL(...) __builtin_ldexpl(__VA_ARGS__)
#else
    #define ZB_MATH_LDEXP(...)  ::std::ldexp(__VA_ARGS__)
    #define ZB_MATH_LDEXPF(...) ::std::ldexpf(__VA_ARGS__)
    #define ZB_MATH_LDEXPL(...) ::std::ldexpl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
namespace zb
{

template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto ldexp(const T arg0, int exp) noexcept
{
    if constexpr (ZB_IS_SAME(T, float))
        return ZB_MATH_LDEXPF(arg0, exp);
    else if constexpr (ZB_IS_SAME(T, double))
        return ZB_MATH_LDEXP(arg0, exp);
    else if constexpr (ZB_IS_SAME(T, long double))
        return ZB_MATH_LDEXPL(arg0, exp);
    else
        static_assert(false);
}

} // namespace zb
