#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::base
{
////////////////////////////////////////////////////////////
template <decltype(sizeof(int))...>
struct IndexSequence
{
};

} // namespace sf::base
