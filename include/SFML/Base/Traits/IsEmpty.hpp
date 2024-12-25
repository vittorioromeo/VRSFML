#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_empty)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_EMPTY(...) __is_empty(__VA_ARGS__)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_EMPTY(...) ::std::is_empty_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isEmpty = SFML_BASE_IS_EMPTY(T);

} // namespace sf::base
