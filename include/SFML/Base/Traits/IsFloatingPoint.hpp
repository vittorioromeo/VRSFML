#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Traits/IsSame.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
#define SFML_BASE_IS_FLOATING_POINT(...)                                                \
    (SFML_BASE_IS_SAME(__VA_ARGS__, float) || SFML_BASE_IS_SAME(__VA_ARGS__, double) || \
     SFML_BASE_IS_SAME(__VA_ARGS__, long double))


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isFloatingPoint = SFML_BASE_IS_FLOATING_POINT(T);

} // namespace sf::base
