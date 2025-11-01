#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_reference)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_REFERENCE(...) __is_reference(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_REFERENCE(...) ::std::is_reference_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isReference = SFML_BASE_IS_REFERENCE(T);

} // namespace sf::base
