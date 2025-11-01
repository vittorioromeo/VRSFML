#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_trivial)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIAL(...) __is_trivial(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
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
