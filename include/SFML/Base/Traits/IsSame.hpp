#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


#if __has_builtin(__is_same)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_SAME(a, b) __is_same(a, b)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename, typename>
inline constexpr bool isSameImpl = false;

template <typename T>
inline constexpr bool isSameImpl<T, T> = true;

} // namespace sf::base::priv

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_SAME(a, b) ::sf::base::priv::isSameImpl<a, b>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename A, typename B>
inline constexpr bool isSame = SFML_BASE_IS_SAME(A, B);

} // namespace sf::base
