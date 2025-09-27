#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool isRvalueReference = false;


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReference<T&&> = true;

} // namespace sf::base
