#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
template <bool, typename T = void>
struct EnableIfImpl
{
};


////////////////////////////////////////////////////////////
template <typename T>
struct EnableIfImpl<true, T>
{
    using type = T;
};


////////////////////////////////////////////////////////////
template <bool B, typename T = void>
using EnableIf = typename EnableIfImpl<B, T>::type;

} // namespace sf::base
