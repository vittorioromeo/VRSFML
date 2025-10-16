#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_trivially_assignable)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(...) __is_trivially_assignable(__VA_ARGS__&, const __VA_ARGS__&)

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(...) ::std::is_trivially_copy_assignable_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyCopyAssignable = SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(T);

} // namespace sf::base
