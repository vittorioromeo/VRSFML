#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_constructible)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_MOVE_CONSTRUCTIBLE(...) __is_constructible(__VA_ARGS__, __VA_ARGS__&&)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_MOVE_CONSTRUCTIBLE(...) ::std::is_move_constructible_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isMoveConstructible = ZB_IS_MOVE_CONSTRUCTIBLE(T);

} // namespace zb
