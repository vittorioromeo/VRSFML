#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__underlying_type)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_UNDERLYING_TYPE(...) __underlying_type(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define SFML_BASE_UNDERLYING_TYPE(...) typename ::std::underlying_type<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using UnderlyingType = SFML_BASE_UNDERLYING_TYPE(T);

} // namespace sf::base
