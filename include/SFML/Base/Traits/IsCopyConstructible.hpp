#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_constructible)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_COPY_CONSTRUCTIBLE(...) __is_constructible(__VA_ARGS__, const __VA_ARGS__&)

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_COPY_CONSTRUCTIBLE(...) ::std::is_copy_constructible_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isCopyConstructible = SFML_BASE_IS_COPY_CONSTRUCTIBLE(T);

} // namespace sf::base
