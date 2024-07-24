#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


#if __has_builtin(__is_trivially_copyable)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_COPYABLE(...) __is_trivially_copyable(__VA_ARGS__)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_COPYABLE(...) ::std::is_trivially_copyable<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyCopyable = SFML_BASE_IS_TRIVIALLY_COPYABLE(T);

} // namespace sf::base
