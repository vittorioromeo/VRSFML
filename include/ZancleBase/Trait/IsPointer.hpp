#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_pointer)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_POINTER(...) __is_pointer(__VA_ARGS__)

#else

namespace zb::priv
{
////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool isPointerImpl = false;

////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isPointerImpl<T*> = true;

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_IS_POINTER(...) ::zb::priv::isPointerImpl<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isPointer = ZB_IS_POINTER(T);

} // namespace zb
