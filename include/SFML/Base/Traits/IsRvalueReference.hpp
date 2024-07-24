#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool isRvalueReference = false;

////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReference<T&&> = true;

} // namespace sf::base
