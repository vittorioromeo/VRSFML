#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
