#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_nothrow_assignable)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(...) __is_nothrow_assignable(__VA_ARGS__&, __VA_ARGS__&&)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(...) ::std::is_nothrow_move_assignable_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isNoThrowMoveAssignable = SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(T);

} // namespace sf::base
