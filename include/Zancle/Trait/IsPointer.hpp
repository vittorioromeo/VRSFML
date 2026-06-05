#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_pointer)

    ////////////////////////////////////////////////////////////
    #define ZA_IS_POINTER(...) __is_pointer(__VA_ARGS__)

#else

namespace za::priv
{
////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool isPointerImpl = false;

////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isPointerImpl<T*> = true;

} // namespace za::priv

    ////////////////////////////////////////////////////////////
    #define ZA_IS_POINTER(...) ::za::priv::isPointerImpl<__VA_ARGS__>

#endif


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isPointer = ZA_IS_POINTER(T);

} // namespace za
