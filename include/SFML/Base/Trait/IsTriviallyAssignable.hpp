#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_trivially_assignable)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(...) __is_trivially_assignable(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(...) ::std::is_trivially_assignable_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename Target, typename Source>
inline constexpr bool isTriviallyAssignable = SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(Target, Source);

} // namespace sf::base
