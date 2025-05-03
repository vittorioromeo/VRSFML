#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_trivially_destructible)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(...) __is_trivially_destructible(__VA_ARGS__)

#elif __has_builtin(__has_trivial_destructor)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(...) __has_trivial_destructor(__VA_ARGS__)

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(...) ::std::is_trivially_destructible_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyDestructible = SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T);

} // namespace sf::base
