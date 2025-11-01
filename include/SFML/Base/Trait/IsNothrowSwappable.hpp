#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/DeclVal.hpp"
#include "SFML/Base/Swap.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isNoThrowSwappable = noexcept(swap(declVal<T&>(), declVal<T&>()));

} // namespace sf::base


////////////////////////////////////////////////////////////
#define SFML_BASE_IS_NOTHROW_SWAPPABLE(...) ::sf::base::isNoThrowSwappable<__VA_ARGS__>
