#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_rvalue_reference)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_RVALUE_REFERENCE(...) __is_rvalue_reference(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_RVALUE_REFERENCE(...) ::std::is_reference_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReference = SFML_BASE_IS_RVALUE_REFERENCE(T);

} // namespace sf::base
