#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_assignable)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_ASSIGNABLE(...) __is_assignable(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_ASSIGNABLE(...) ::std::is_assignable_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T, typename U>
inline constexpr bool isAssignable = ZB_IS_ASSIGNABLE(T, U);

} // namespace zb
