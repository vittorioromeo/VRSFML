#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_standard_layout)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_STANDARD_LAYOUT(...) __is_standard_layout(__VA_ARGS__)

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_STANDARD_LAYOUT(...) ::std::is_standard_layout_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isStandardLayout = SFML_BASE_IS_STANDARD_LAYOUT(T);

} // namespace sf::base
