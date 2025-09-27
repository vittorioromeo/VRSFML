#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isVoid = false;


////////////////////////////////////////////////////////////
template <>
inline constexpr bool isVoid<void> = true;

} // namespace sf::base
