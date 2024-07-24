#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


namespace sf::base
{
////////////////////////////////////////////////////////////
template <decltype(sizeof(int))...>
struct IndexSequence
{
};

} // namespace sf::base
