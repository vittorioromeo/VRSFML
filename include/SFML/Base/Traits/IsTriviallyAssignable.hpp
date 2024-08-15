#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_trivially_assignable)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(...) __is_trivially_assignable(__VA_ARGS__)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(...) ::std::is_trivially_assignable_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename A, typename B>
inline constexpr bool isTriviallyAssignable = SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(A, B);

} // namespace sf::base
