#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_constructible)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONSTRUCTIBLE(...) __is_constructible(__VA_ARGS__)

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONSTRUCTIBLE(...) ::std::is_constructible_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T, typename... Args>
inline constexpr bool isConstructible = SFML_BASE_IS_CONSTRUCTIBLE(T, Args...);

} // namespace sf::base
