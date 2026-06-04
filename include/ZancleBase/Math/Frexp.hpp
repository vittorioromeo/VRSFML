#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
#include "ZancleBase/Math/Priv/Impl.hpp"
#include "ZancleBase/Trait/IsSame.hpp"


////////////////////////////////////////////////////////////
#if ZB_PRIV_HAS_MATH_BUILTIN(frexp)
    #define ZB_MATH_FREXP(...)  __builtin_frexp(__VA_ARGS__)
    #define ZB_MATH_FREXPF(...) __builtin_frexpf(__VA_ARGS__)
    #define ZB_MATH_FREXPL(...) __builtin_frexpl(__VA_ARGS__)
#else
    #include <cmath> // IWYU pragma: keep

    #define ZB_MATH_FREXP(...)  ::std::frexp(__VA_ARGS__)
    #define ZB_MATH_FREXPF(...) ::std::frexpf(__VA_ARGS__)
    #define ZB_MATH_FREXPL(...) ::std::frexpl(__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////
namespace zb
{

template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto frexp(const T arg0, int* exp) noexcept
{
    if constexpr (ZB_IS_SAME(T, float))
        return ZB_MATH_FREXPF(arg0, exp);
    else if constexpr (ZB_IS_SAME(T, double))
        return ZB_MATH_FREXP(arg0, exp);
    else if constexpr (ZB_IS_SAME(T, long double))
        return ZB_MATH_FREXPL(arg0, exp);
    else
        static_assert(false);
}

} // namespace zb
