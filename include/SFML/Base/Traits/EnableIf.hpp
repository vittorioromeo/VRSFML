#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
