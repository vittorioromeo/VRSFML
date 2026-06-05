#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_assignable)

    ////////////////////////////////////////////////////////////
    #define ZA_IS_MOVE_ASSIGNABLE(...) __is_assignable(__VA_ARGS__&, __VA_ARGS__&&)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZA_IS_MOVE_ASSIGNABLE(...) ::std::is_move_assignable_v<__VA_ARGS__>

#endif


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isMoveAssignable = ZA_IS_MOVE_ASSIGNABLE(T);

} // namespace za
