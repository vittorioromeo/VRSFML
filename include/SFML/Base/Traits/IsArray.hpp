#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_array)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_ARRAY(...) __is_array(__VA_ARGS__)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename>              struct IsArrayHelper          { enum { value = false }; };
template <typename T, auto Size> struct IsArrayHelper<T[Size]> { enum { value = true }; };
template <typename T>            struct IsArrayHelper<T[]>     { enum { value = true }; };
// clang-format on

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_ARRAY(...) ::sf::base::priv::IsArrayHelper<__VA_ARGS__>::value

} // namespace sf::base::priv

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isArray = SFML_BASE_IS_ARRAY(T);

} // namespace sf::base
