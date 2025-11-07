#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_rvalue_reference)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_RVALUE_REFERENCE(...) __is_rvalue_reference(__VA_ARGS__)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReferenceImpl = false;


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReferenceImpl<T&&> = true;

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_RVALUE_REFERENCE(...) ::sf::base::priv::isRvalueReferenceImpl<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReference = SFML_BASE_IS_RVALUE_REFERENCE(T);

} // namespace sf::base
