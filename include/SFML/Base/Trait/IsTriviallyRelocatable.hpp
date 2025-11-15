#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"


namespace sf::base::priv
{
#if __has_builtin(__builtin_is_cpp_trivially_relocatable)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_RELOCATABLE_BY_BUILTIN(...) __builtin_is_cpp_trivially_relocatable(__VA_ARGS__)

#elif __has_builtin(__is_trivially_relocatable)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_RELOCATABLE_BY_BUILTIN(...) __is_trivially_relocatable(__VA_ARGS__)

#else

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_RELOCATABLE_BY_BUILTIN(...) false

#endif


////////////////////////////////////////////////////////////
template <typename T>
inline consteval bool hasEnabledTrivialRelocation()
{
    if constexpr (requires { T::enableTrivialRelocation; })
    {
        return T::enableTrivialRelocation;
    }
    else
    {
        return false;
    }
}

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool enableTrivialRelocation = priv::hasEnabledTrivialRelocation<T>();

} // namespace sf::base


////////////////////////////////////////////////////////////
#define SFML_BASE_IS_TRIVIALLY_RELOCATABLE(...)                                                                    \
    (SFML_BASE_IS_TRIVIALLY_RELOCATABLE_BY_BUILTIN(__VA_ARGS__) || SFML_BASE_IS_TRIVIALLY_COPYABLE(__VA_ARGS__) || \
     ::sf::base::enableTrivialRelocation<__VA_ARGS__>)


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyRelocatable = SFML_BASE_IS_TRIVIALLY_RELOCATABLE(T);

} // namespace sf::base
