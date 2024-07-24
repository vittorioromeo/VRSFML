#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/Traits/IsSame.hpp>


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isFloatingPoint = SFML_BASE_IS_SAME(T, float) || SFML_BASE_IS_SAME(T, double) ||
                                        SFML_BASE_IS_SAME(T, long double);

} // namespace sf::base
