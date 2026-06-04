#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_nothrow_assignable)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_NOTHROW_MOVE_ASSIGNABLE(...) __is_nothrow_assignable(__VA_ARGS__&, __VA_ARGS__&&)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_NOTHROW_MOVE_ASSIGNABLE(...) ::std::is_nothrow_move_assignable_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isNoThrowMoveAssignable = ZB_IS_NOTHROW_MOVE_ASSIGNABLE(T);

} // namespace zb
