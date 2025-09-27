#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_convertible)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONVERTIBLE(from, to) __is_convertible(from, to)

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONVERTIBLE(from, to) ::std::is_convertible_v<from, to>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename From, typename To>
inline constexpr bool isConvertible = SFML_BASE_IS_CONVERTIBLE(From, To);

} // namespace sf::base
