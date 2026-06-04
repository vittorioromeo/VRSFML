#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_member_pointer)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_MEMBER_POINTER(...) __is_member_pointer(__VA_ARGS__)

#else

namespace zb::priv
{
////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool isMemberPointerImpl = false;

////////////////////////////////////////////////////////////
template <typename T, typename C>
inline constexpr bool isMemberPointerImpl<T C::*> = true;

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_IS_MEMBER_POINTER(...) ::zb::priv::isMemberPointerImpl<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isMemberPointer = ZB_IS_MEMBER_POINTER(T);

} // namespace zb
