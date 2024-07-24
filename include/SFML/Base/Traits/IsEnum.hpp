#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


#if __has_builtin(__is_enum)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_ENUM(...) __is_enum(__VA_ARGS__)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_ENUM(...) ::std::is_enum_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isEnum = SFML_BASE_IS_ENUM(T);

} // namespace sf::base
