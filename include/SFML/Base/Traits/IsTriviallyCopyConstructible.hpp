#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


#if __has_builtin(__is_trivially_constructible)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(...) __is_trivially_constructible(__VA_ARGS__, const __VA_ARGS__&)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(...) ::std::is_trivially_copy_constructible<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyCopyConstructible = SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T);

} // namespace sf::base
