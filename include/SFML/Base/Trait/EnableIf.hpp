#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base::priv
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

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <bool B, typename T = void>
using EnableIf = typename priv::EnableIfImpl<B, T>::type;

} // namespace sf::base
