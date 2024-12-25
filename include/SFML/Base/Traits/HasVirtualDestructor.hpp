#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__has_virtual_destructor)

////////////////////////////////////////////////////////////
#define SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(...) __has_virtual_destructor(__VA_ARGS__)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(...) ::std::has_virtual_destructor_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool hasVirtualDestructor = SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(T);

} // namespace sf::base
