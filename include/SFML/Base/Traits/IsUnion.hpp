#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_union)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_UNION(...) __is_union(__VA_ARGS__)

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_UNION(...) ::std::is_union_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isUnion = SFML_BASE_IS_UNION(T);

} // namespace sf::base
