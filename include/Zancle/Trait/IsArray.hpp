#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_array)

    ////////////////////////////////////////////////////////////
    #define ZA_IS_ARRAY(...) __is_array(__VA_ARGS__)

#else

namespace za::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename>              struct IsArrayImpl          { enum { value = false }; };
template <typename T, auto Size> struct IsArrayImpl<T[Size]> { enum { value = true }; };
template <typename T>            struct IsArrayImpl<T[]>     { enum { value = true }; };
// clang-format on

    ////////////////////////////////////////////////////////////
    #define ZA_IS_ARRAY(...) ::za::priv::IsArrayImpl<__VA_ARGS__>::value

} // namespace za::priv

#endif


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isArray = ZA_IS_ARRAY(T);

} // namespace za
