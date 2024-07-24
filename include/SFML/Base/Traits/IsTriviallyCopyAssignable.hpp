#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


#if __has_builtin(__is_trivially_assignable)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(...) __is_trivially_assignable(__VA_ARGS__&, const __VA_ARGS__&)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(...) ::std::is_trivially_copy_assignable<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyCopyAssignable = SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(T);

} // namespace sf::base
