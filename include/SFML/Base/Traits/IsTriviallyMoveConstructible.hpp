#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_trivially_constructible)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(...) __is_trivially_constructible(__VA_ARGS__, __VA_ARGS__&&)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(...) ::std::is_trivially_move_constructible_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyMoveConstructible = SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(T);

} // namespace sf::base