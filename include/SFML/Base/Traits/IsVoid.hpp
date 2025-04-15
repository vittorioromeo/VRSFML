#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isVoid = false;


////////////////////////////////////////////////////////////
template <>
inline constexpr bool isVoid<void> = true;

} // namespace sf::base
