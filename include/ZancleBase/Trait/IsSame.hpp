#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_same)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_SAME(a, b) __is_same(a, b)

#else

namespace zb::priv
{
////////////////////////////////////////////////////////////
template <typename, typename>
inline constexpr bool isSameImpl = false;


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isSameImpl<T, T> = true;

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_IS_SAME(a, b) ::zb::priv::isSameImpl<a, b>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename A, typename B>
inline constexpr bool isSame = ZB_IS_SAME(A, B);

} // namespace zb
