#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_constructible)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(...) __is_constructible(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(...) ::std::is_default_constructible_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isDefaultConstructible = SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(T);

} // namespace sf::base
