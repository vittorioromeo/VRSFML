#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_array)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_ARRAY(...) __is_array(__VA_ARGS__)

#else

namespace zb::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename>              struct IsArrayImpl          { enum { value = false }; };
template <typename T, auto Size> struct IsArrayImpl<T[Size]> { enum { value = true }; };
template <typename T>            struct IsArrayImpl<T[]>     { enum { value = true }; };
// clang-format on

    ////////////////////////////////////////////////////////////
    #define ZB_IS_ARRAY(...) ::zb::priv::IsArrayImpl<__VA_ARGS__>::value

} // namespace zb::priv

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isArray = ZB_IS_ARRAY(T);

} // namespace zb
