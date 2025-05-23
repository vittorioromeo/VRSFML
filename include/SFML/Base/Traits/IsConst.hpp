#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_const)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONST(...) __is_const(__VA_ARGS__)

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONST(...) ::std::is_const_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isConst = SFML_BASE_IS_CONST(T);

} // namespace sf::base
