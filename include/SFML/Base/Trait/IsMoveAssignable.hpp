#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_assignable)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_MOVE_ASSIGNABLE(...) __is_assignable(__VA_ARGS__&, __VA_ARGS__&&)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_MOVE_ASSIGNABLE(...) ::std::is_move_assignable_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isMoveAssignable = SFML_BASE_IS_MOVE_ASSIGNABLE(T);

} // namespace sf::base
