#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_unsigned)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_UNSIGNED(...) __is_unsigned(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include "SFML/Base/Trait/IsIntegral.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T, bool = isIntegral<T>>
inline constexpr bool isUnsignedImpl = false;


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isUnsignedImpl<T, true> = T(0) < T(-1);

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_UNSIGNED(...) ::sf::base::priv::isUnsignedImpl<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isUnsigned = SFML_BASE_IS_UNSIGNED(T);

} // namespace sf::base
