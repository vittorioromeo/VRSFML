#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_pointer)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_POINTER(...) __is_pointer(__VA_ARGS__)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool isPointerImpl = false;

////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isPointerImpl<T*> = true;

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_POINTER(...) ::sf::base::priv::isPointerImpl<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isPointer = SFML_BASE_IS_POINTER(T);

} // namespace sf::base
