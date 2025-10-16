#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_member_pointer)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_MEMBER_POINTER(...) __is_member_pointer(__VA_ARGS__)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool isMemberPointerImpl = false;

////////////////////////////////////////////////////////////
template <typename T, typename C>
inline constexpr bool isMemberPointerImpl<T C::*> = true;

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_MEMBER_POINTER(...) ::sf::base::priv::isMemberPointerImpl<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isMemberPointer = SFML_BASE_IS_MEMBER_POINTER(T);

} // namespace sf::base
