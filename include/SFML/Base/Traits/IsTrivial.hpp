#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_trivial)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIAL(...) __is_trivial(__VA_ARGS__)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIAL(...) ::std::is_trivial_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTrivial = SFML_BASE_IS_TRIVIAL(T);

} // namespace sf::base
