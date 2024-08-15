#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_nothrow_swappable)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_NOTHROW_SWAPPABLE(...) __is_nothrow_swappable(__VA_ARGS__)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_NOTHROW_SWAPPABLE(...) ::std::is_nothrow_swappable_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isNoThrowSwappable = SFML_BASE_IS_NOTHROW_SWAPPABLE(T);

} // namespace sf::base
